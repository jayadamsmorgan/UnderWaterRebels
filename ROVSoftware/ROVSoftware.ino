#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>
#include <SparkFun_MS5803_I2C.h>
#include <ADXL345.h>
#include <HMC5883L.h>

typedef unsigned uint;
typedef unsigned char uchar;

#define MOTOR1PIN                45
#define MOTOR2PIN                46
#define MOTOR3PIN                44
#define MOTOR4PIN                11
#define MOTOR5PIN                13
#define MOTOR6PIN                12

#define MOTORLOWMICROSECONDS     1465
#define MOTORHIGHMICROSECONDS    1510
#define MOTORRANGE               400

#define HIGH_SPEED_K             1.0
#define MID_SPEED_K              0.6
#define LOW_SPEED_K              0.3

#define MAIN_MANIP_ROT_PINA      9
#define MAIN_MANIP_ROT_PINB      14

#define MAIN_MANIP_TIGHT_PINA    28
#define MAIN_MANIP_TIGHT_PINB    29

#define MULTIPLEXOR_PIN          24

#define LED_PIN                  26

#define SERVO_MANIPULATOR_PIN    8
#define SERVO_CAMERA_PIN         4

#define SERVO_UPDATE_WINDOW      30   // Delay for updating servo's angle
#define SERVO_ANGLE_DELTA        3

#define MIN_CAMERA_ANGLE         20
#define MAX_CAMERA_ANGLE         160

#define MAX_BOTTOM_MANIP_ANGLE   80
#define MIN_BOTTOM_MANIP_ANGLE   0

#define INCOMING_PACKET_SIZE     25
#define OUTCOMING_PACKET_SIZE    15

double PITCH_KP =               2.0;
double PITCH_KI =               0.0;
double PITCH_KD =               0.0;

double DEPTH_KP =               2.0;
double DEPTH_KI =               0.0;
double DEPTH_KD =               0.0;

double YAW_KP1  =               0.0001;
double YAW_KP   =               2.0;
double YAW_KI   =               0.0;
double YAW_KD   =               0.0;

int MOTORMIDMICROSECONDS = (MOTORLOWMICROSECONDS + MOTORHIGHMICROSECONDS) / 2.0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177), remote_device;
char packetBuffer[INCOMING_PACKET_SIZE];
unsigned char replyBuffer[OUTCOMING_PACKET_SIZE];
EthernetUDP Udp;

ADXL345 accelerometer;
HMC5883L compass;
MS5803 sensor(ADDRESS_HIGH);

Servo horMotor1, horMotor2, horMotor3, horMotor4;
Servo verMotor1, verMotor2;

Servo camera, bottomManip;
int camera_angle, new_camera_angle, bottom_manip_angle, new_bottom_manip_angle;
unsigned long long prev_camera_servo_update, prev_manip_servo_update;

char servoCamDir = 0, manTightDir = 0, botManipDir = 0;

double declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
double yaw = 0, pitch = 0, roll = 0;
int depth = 0;

// Create variables to store results for depth calculations
double pressure_abs, pressure_baseline;

// Variable for storing joystick values
signed char js_val[5];
bool buttons[8];

// Auto modes
bool isAutoDepth = false, isAutoPitch = false, isAutoYaw = false;

// LED switch
bool isLED = false;

// Mux channels
unsigned char muxChannel = 0;

// Array for leak sensors values
bool leak[8];

// Speed mode for arranging speed
double speedK = 0.3;

// PIDs for auto modes
double pitchSetpoint, pitchInput, pitchOutput;
PID autoPitchPID(&pitchInput, &pitchOutput, &pitchSetpoint, PITCH_KP, PITCH_KI, PITCH_KD, DIRECT);
double depthSetpoint, depthInput, depthOutput;
PID autoDepthPID(&depthInput, &depthOutput, &depthSetpoint, DEPTH_KP, DEPTH_KI, DEPTH_KD, DIRECT);
double yawSetpoint, yawInput, yawOutput;
PID autoYawPID(&yawInput, &yawOutput, &yawSetpoint, YAW_KP1, YAW_KI, YAW_KD, DIRECT);

// Function for controlling motor system
void controlPeripherals() {
  // Auto modes realization:
  if (isAutoPitch && isAutoDepth && js_val[2] == 0) {
    Serial.println("Using AutoPitch & AutoDepth mode");
    autoPitchAndDepth();
  } else if (isAutoPitch && js_val[2] == 0) {
    Serial.println("Using AutoPitch mode");
    autoPitch();
    depthSetpoint = depth; // Set target for AutoDepth
  } else if (isAutoDepth && js_val[2] == 0) {
    Serial.println("Using AutoDepth mode");
    autoDepth();
  } else {
    // Set vertical thrust
    verticalMotorControl(verMotor1, -js_val[2]);
    verticalMotorControl(verMotor2, js_val[2]);

    // Set target for AutoDepth
    depthSetpoint = depth;
  }

  // AutoYaw mode realization:
  if (isAutoYaw && js_val[0] == 0 && js_val[1] == 0 && js_val[3] == 0) {
    Serial.println("Using AutoYaw mode");
    autoYaw();
  } else {
    // Set horizontal thrust
    horizontalMotorControl(horMotor1, js_val[0], -js_val[1], -js_val[3]);
    horizontalMotorControl(horMotor2, js_val[0], js_val[1], -js_val[3]);
    horizontalMotorControl(horMotor3, js_val[0], -js_val[1], js_val[3]);
    horizontalMotorControl(horMotor4, js_val[0], js_val[1], js_val[3]);

    // Set target for AutoYaw
    yawSetpoint = yaw;
  }

  // Rotate & tight manipulator
  rotateManipulator(js_val[4]);
  tightenManipulator(manTightDir);

  // Camera's servo controlling
  unsigned long long current_time = millis();
  
  if (servoCamDir != 0 && (current_time - prev_camera_servo_update >= SERVO_UPDATE_WINDOW)) {
    if (servoCamDir > 0) {
      new_camera_angle += SERVO_ANGLE_DELTA;
      if (new_camera_angle > MAX_CAMERA_ANGLE)
        new_camera_angle = MAX_CAMERA_ANGLE;
    } else {
      new_camera_angle -= SERVO_ANGLE_DELTA;
      if (new_camera_angle < MIN_CAMERA_ANGLE)
        new_camera_angle = MIN_CAMERA_ANGLE;
    }
  }
  if (camera_angle != new_camera_angle) {
    camera.write(new_camera_angle);
    camera_angle = new_camera_angle;
  }
  prev_camera_servo_update = millis();

  // Bottom maniplulator's servo controlling
  current_time = millis();
  if (botManipDir != 0 && (current_time - prev_manip_servo_update >= SERVO_UPDATE_WINDOW)) {
    if (botManipDir > 0) {
      new_bottom_manip_angle += SERVO_ANGLE_DELTA;
      if (new_bottom_manip_angle > MAX_BOTTOM_MANIP_ANGLE)
        new_bottom_manip_angle = MAX_BOTTOM_MANIP_ANGLE;
    } else {
      new_bottom_manip_angle -= SERVO_ANGLE_DELTA;
      if (new_bottom_manip_angle < MIN_BOTTOM_MANIP_ANGLE)
        new_bottom_manip_angle = MIN_BOTTOM_MANIP_ANGLE;
    }
  }
  if (bottom_manip_angle != new_bottom_manip_angle) {
    bottomManip.write(new_bottom_manip_angle);
    bottom_manip_angle = new_bottom_manip_angle;
  }
  prev_manip_servo_update = millis();

  // Select multiplexor channel for right video out
  selectMuxChannel();

  // Switch on/off LED
  switchLED();
}

// AutoPitch & AutoDepth mode
void autoPitchAndDepth() {

  depthInput = depthSetpoint - depth;
  pitchInput = pitch;

  autoPitchPID.Compute();
  autoDepthPID.Compute();

  double output1, output2;

  output1 = (depthOutput + pitchOutput) / 1.5;
  output2 = (depthOutput - pitchOutput) / 1.5;

  // Value correction:
  if (output1 > 100.0) {
    output1 = 100.0;
  }
  if (output1 < -100.0) {
    output1 = -100.0;
  }
  if (output2 > 100.0) {
    output2 = 100.0;
  }
  if (output2 < -100.0) {
    output2 = -100.0;
  }

  verticalMotorControl(verMotor1, (char) - output1);
  verticalMotorControl(verMotor2, (char) output2);
}

// AutoPitch mode
void autoPitch() {
  // Some PID magic, some bad coding....... Do not try to understand that.......
  pitchInput = pitch;
  Serial.print("Rotation angle: "); Serial.println(pitchInput);
  signed char dir = 0;
  if (pitchInput > 0) {
    dir = 1;
  } else {
    dir = -1;
  }
  pitchInput = -abs(pitchInput);
  autoPitchPID.Compute();

  if (dir < 0) {
    pitchOutput = -abs(pitchOutput);
  }
  // ...........

  // Value correction:
  if (pitchOutput > 100.0) {
    pitchOutput = 100.0;
  }
  if (pitchOutput < -100.0) {
    pitchOutput = -100.0;
  }
  Serial.print("AutoPitch PID output is: "); Serial.println(pitchOutput);
  Serial.print("Target pitch is: ");         Serial.println(pitchSetpoint);
  Serial.print("Current pitch is: ");        Serial.println(pitch);

  verticalMotorControl(verMotor1, (char) - pitchOutput);
  verticalMotorControl(verMotor2, (char) - pitchOutput);
}

// AutoDepth mode
void autoDepth() {
  depthSetpoint = -40;
  depthInput = depthSetpoint - depth;
  signed char dir = 0;
  if (depthInput > 0) {
    dir = 1;
  } else {
    dir = -1;
  }
  depthInput = -abs(depthInput);
  autoDepthPID.Compute();
  Serial.print("AutoDepth PID output is: "); Serial.println(depthOutput);
  Serial.print("Target depth is: ");         Serial.println(depthSetpoint);
  Serial.print("Current depth is: ");        Serial.println(depth);

  if (dir < 0) {
    depthOutput = -abs(depthOutput);
  }

  // Value correction:
  if (depthOutput > 100.0) {
    depthOutput = 100.0;
  }
  if (depthOutput < -100.0) {
    depthOutput = -100.0;
  }

  verticalMotorControl(verMotor1, (char) - depthOutput);
  verticalMotorControl(verMotor2, (char) depthOutput);
}

// AutoYaw mode
void autoYaw() {
  // Some PID magic, some bad coding....... Do not try to understand that.......
  yawInput = rotationAngle(yaw, yawSetpoint);
  Serial.print("Rotation angle: "); Serial.println(yawInput);
  if (yawInput > -0.50 && yawInput < 0.50) {
    return;
  }
  signed char dir = 0;
  if (yawInput > 0) {
    dir = 1;
  } else {
    dir = -1;
  }
  yawInput = -abs(yawInput);
  autoYawPID.Compute();
  if (dir < 0) {
    yawOutput = -abs(yawOutput);
  }
  double val = yawOutput * 5000;
  if (dir == 0) {
    val = 0;
  }
  if (dir > 0) {
    val -= yawSetpoint / 2;
  }
  if (dir < 0) {
    val += yawSetpoint / 2;
  }
  val *= YAW_KP;
  // .......

  Serial.print("AutoYaw PID output is: "); Serial.println(val);
  Serial.print("Target yaw is: ");         Serial.println(yawSetpoint);
  Serial.print("Current yaw is: ");        Serial.println(yaw);

  horizontalMotorControl(horMotor1, 0, 0, val);
  horizontalMotorControl(horMotor2, 0, 0, -val);
  horizontalMotorControl(horMotor3, 0, 0, val);
  horizontalMotorControl(horMotor4, 0, 0, -val);
}

// Function for correct angles for PID (calcuating minimal angle for rotation)
double rotationAngle(double currentAngle, double targetAngle) {
  double rotationAngle = currentAngle - targetAngle;
  if (rotationAngle >= 180.00) {
    rotationAngle = rotationAngle - 360.00;
  } else if (rotationAngle < -180.00) {
    rotationAngle = 360.00 - abs(rotationAngle);
  }
  return rotationAngle; // -180 < rotationAngle <= 180
}

// Receiving messages from PC & parsing
char receiveMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    Serial.print("Received packet of size: ");
    Serial.println(packetSize);
  }
  if (packetSize == INCOMING_PACKET_SIZE) {
    Serial.println("Size is correct.");
    remote_device = Udp.remoteIP();
    Udp.read(packetBuffer, INCOMING_PACKET_SIZE);

    // STARTING PARSING PACKET ********
    for (int i = 0; i < 5; ++i)  {
      js_val[i] = (signed char)packetBuffer[i];
      Serial.print("js"); Serial.print(i); Serial.print(": "); Serial.println(js_val[i]);
    }
    js_val[2] = -js_val[2];
    for (int i = 0; i < 8; ++i) {
      buttons[i] = (packetBuffer[5] >> i) & 1;
      Serial.print("btn"); Serial.print(i); Serial.print(": "); Serial.println(buttons[i]);
    }

    if (buttons[0] == 0 && buttons[1] == 1) {
      servoCamDir = 1;
    } else if (buttons[0] == 1 && buttons[1] == 0) {
      servoCamDir = -1;
    } else {
      servoCamDir = 0;
    }

    if (buttons[2] == 0 && buttons[3] == 1) {
      manTightDir = -1;
    } else if (buttons[2] == 1 && buttons[3] == 0) {
      manTightDir = 1;
    } else {
      manTightDir = 0;
    }

    if (buttons[4] == 0 && buttons[5] == 1) {
      botManipDir = -1;
    } else if (buttons[4] == 1 && buttons[5] == 0) {
      botManipDir = 1;
    } else {
      botManipDir = 0;
    }

    if (buttons[6] == 1) {
      muxChannel = 1;
    } else {
      muxChannel = 0;
    }

    char bit1 = (packetBuffer[6]) & 1;
    char bit2 = (packetBuffer[6] >> 1) & 1;
    char bit3 = (packetBuffer[6] >> 2) & 1;

    if (bit1) {
      Serial.println("Using high-speed mode");
      speedK = HIGH_SPEED_K;
    } else if (bit2) {
      Serial.println("Using mid-speed mode");
      speedK = MID_SPEED_K;
    } else if (bit3) {
      Serial.println("Using low-speed mode");
      speedK = LOW_SPEED_K;
    } else {
      Serial.println("Using mid-speed mode");
      speedK = MID_SPEED_K;
    }

    isAutoPitch = (packetBuffer[6] >> 3) & 1;
    isAutoDepth = (packetBuffer[6] >> 4) & 1;
    isAutoYaw = (packetBuffer[6] >> 5) & 1;

    isLED = (packetBuffer[6] >> 6) & 1;
    Serial.print("isLED: "); Serial.println(isLED);

    //getK();
    // ENDING PARSING PACKET ********

    //setK();

    return 1;
  } else {
    return 0;
  }
}

// Function to get koefficients from packetBuffer
void getK() {
  YAW_KP = (double) bytesToUInt(packetBuffer[7], packetBuffer[8]) / 10000;
  YAW_KI = (double) bytesToUInt(packetBuffer[9], packetBuffer[10]) / 10000;
  YAW_KD = (double) bytesToUInt(packetBuffer[11], packetBuffer[12]) / 10000;
  PITCH_KP = (double) bytesToUInt(packetBuffer[13], packetBuffer[14]) / 10000;
  PITCH_KI = (double) bytesToUInt(packetBuffer[15], packetBuffer[16]) / 10000;
  PITCH_KD = (double) bytesToUInt(packetBuffer[17], packetBuffer[18]) / 10000;
  DEPTH_KP = (double) bytesToUInt(packetBuffer[19], packetBuffer[20]) / 10000;
  DEPTH_KI = (double) bytesToUInt(packetBuffer[21], packetBuffer[22]) / 10000;
  DEPTH_KD = (double) bytesToUInt(packetBuffer[23], packetBuffer[24]) / 10000;
}

int bytesToUInt(byte firstByte, byte secondByte) {
  return (static_cast<uint>(static_cast<uchar>(secondByte)) << 8 ) | static_cast<uint>(static_cast<uchar>(firstByte));
}

// Function to set values for PIDs
void setK() {
  autoPitchPID.SetTunings(PITCH_KP, PITCH_KI, PITCH_KD);
  autoYawPID.SetTunings(YAW_KP1, YAW_KI, YAW_KD);
  autoDepthPID.SetTunings(DEPTH_KP, DEPTH_KI, DEPTH_KD);
}

// Forming & sending packet to PC via UDP
void sendReply() {
  Serial.print("PC is on :"); Serial.println(remote_device);

  Serial.println("Forming packet...");
  replyBuffer[0]  = ((int) (yaw * 100.00) >> 8) & 0xFF;
  replyBuffer[1]  = ((int) (yaw * 100.00)) & 0xFF;
  replyBuffer[2]  = ((int) (pitch * 100.00) >> 8) & 0xFF;
  replyBuffer[3]  = ((int) (pitch * 100.00)) & 0xFF;
  replyBuffer[4]  = ((int) (roll * 100.00) >> 8) & 0xFF;
  replyBuffer[5]  = ((int) (roll * 100.00)) & 0xFF;
  replyBuffer[6]  = ((int) (depth) >> 8) & 0xFF;
  replyBuffer[7]  = ((int) (depth)) & 0xFF;
  replyBuffer[8]  = ((int) (yawSetpoint * 100.00) >> 8) & 0xFF;
  replyBuffer[9]  = ((int) (yawSetpoint * 100.00)) & 0xFF;
  replyBuffer[10] = ((int) (depthSetpoint) >> 8 ) & 0xFF;
  replyBuffer[11] = ((int) (depthSetpoint)) & 0xFF;
  for (int i = 0; i < 8; ++i) {
    replyBuffer[12] |= leak[i] << i;
  }
  replyBuffer[13] = ((uint) (new_camera_angle) >> 8) & 0xFF;
  replyBuffer[14] = ((uint) (new_camera_angle)) & 0xFF;

  Serial.println("Replying...");
  Udp.beginPacket(remote_device, Udp.remotePort());
  Serial.println(Udp.write(replyBuffer, OUTCOMING_PACKET_SIZE));
  Serial.println("Writing packet...");
  Udp.endPacket();
  Serial.println("Endpacket...");
  return;
}

// Function to control horizontal brushless motors
void horizontalMotorControl(Servo motor, short x, short y, short z) {
  int POW = 0;
  int sum = x + y + z;
  if (sum > 100.0) sum = 100.00;
  if (sum < (-100.0)) sum = -100.00;
  POW = int((sum * (MOTORRANGE / 100.0)) * speedK);
  Serial.print("Horizontal motor pow: "); Serial.println(POW);
  if (POW == 0) {
    motor.writeMicroseconds(MOTORMIDMICROSECONDS);
  }
  if (POW < 0) {
    motor.writeMicroseconds(MOTORLOWMICROSECONDS + POW);
  }
  if (POW > 0) {
    motor.writeMicroseconds(MOTORHIGHMICROSECONDS + POW);
  }
}

// Function to control vertical brushless motors
void verticalMotorControl(Servo motor, short z) {
  int POW = 0;
  int sum = z;
  if (sum > 100.0) sum = 100.0;
  if (sum < (-100.0)) sum = -100.0;
  POW = int((sum * (MOTORRANGE / 100.0)) * speedK);
  Serial.print("Vertical motor pow: "); Serial.println(POW);
  if (POW == 0) {
    motor.writeMicroseconds(MOTORMIDMICROSECONDS);
  }
  if (POW < 0) {
    motor.writeMicroseconds(MOTORLOWMICROSECONDS + POW);
  }
  if (POW > 0) {
    motor.writeMicroseconds(MOTORHIGHMICROSECONDS + POW);
  }
}

// Function to rotate manipulator
void rotateManipulator(short m) {
  if (m > 0) {
    digitalWrite(MAIN_MANIP_ROT_PINA, HIGH);
    digitalWrite(MAIN_MANIP_ROT_PINB, LOW);
  }
  if (m < 0) {
    digitalWrite(MAIN_MANIP_ROT_PINA, LOW);
    digitalWrite(MAIN_MANIP_ROT_PINB, HIGH);
  }
  if (m == 0) {
    digitalWrite(MAIN_MANIP_ROT_PINA, LOW);
    digitalWrite(MAIN_MANIP_ROT_PINB, LOW);
  }
}

// Function to tight manipulator
void tightenManipulator(char dir) {
  if (dir > 0) {
    digitalWrite(MAIN_MANIP_TIGHT_PINA, HIGH);
    digitalWrite(MAIN_MANIP_TIGHT_PINB, LOW);
  }
  if (dir < 0) {
    digitalWrite(MAIN_MANIP_TIGHT_PINA, LOW);
    digitalWrite(MAIN_MANIP_TIGHT_PINB, HIGH);
  }
  if (dir == 0) {
    digitalWrite(MAIN_MANIP_TIGHT_PINA, LOW);
    digitalWrite(MAIN_MANIP_TIGHT_PINB, LOW);
  }
}

// Setup function
void setup() {
  // Init I2C connection for IMU
  Wire.begin();

  // Init serial port for debugging
  Serial.begin(250000);

  // Init brushless motors
  horMotor1.attach(MOTOR1PIN);
  horMotor2.attach(MOTOR2PIN);
  horMotor3.attach(MOTOR3PIN);
  horMotor4.attach(MOTOR4PIN);
  verMotor1.attach(MOTOR5PIN);
  verMotor2.attach(MOTOR6PIN);
  delay(3000);
  horMotor1.writeMicroseconds(MOTORMIDMICROSECONDS);
  horMotor2.writeMicroseconds(MOTORMIDMICROSECONDS);
  horMotor3.writeMicroseconds(MOTORMIDMICROSECONDS);
  horMotor4.writeMicroseconds(MOTORMIDMICROSECONDS);
  verMotor1.writeMicroseconds(MOTORMIDMICROSECONDS);
  verMotor2.writeMicroseconds(MOTORMIDMICROSECONDS);
  delay(5000);

  // Ethernet & Serial port init
  Ethernet.begin(mac, ip);
  Udp.begin(8000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(MAIN_MANIP_ROT_PINA, OUTPUT);
  pinMode(MAIN_MANIP_ROT_PINB, OUTPUT);
  pinMode(MAIN_MANIP_TIGHT_PINA, OUTPUT);
  pinMode(MAIN_MANIP_TIGHT_PINB, OUTPUT);
  pinMode(SERVO_CAMERA_PIN, OUTPUT);
  pinMode(SERVO_MANIPULATOR_PIN, OUTPUT);

  // Init bottom manipulator & main camera
  camera.attach(SERVO_CAMERA_PIN);
  new_camera_angle = 120;
  camera.write(new_camera_angle);
  bottomManip.attach(SERVO_MANIPULATOR_PIN);
  new_bottom_manip_angle = 80;
  bottomManip.write(new_bottom_manip_angle);

  // Init PID settings
  autoPitchPID.SetMode(AUTOMATIC);
  autoDepthPID.SetMode(AUTOMATIC);
  autoYawPID.SetMode(AUTOMATIC);
  pitchSetpoint = 0;
  accelerometer.begin();
  compass.begin();

  // Some delay for motors...
  delay(1000);
  // Retrieve calibration constants for conversion math.
  sensor.reset();
  sensor.begin();
  pressure_baseline = sensor.getPressure(ADC_4096);
}

// Function for updating depth
void updateDepth() {
  // Read pressure from the sensor in mbar & filter it
  int pressureReadings[10];
  for (int i = 0; i < 10; i++) {
    pressureReadings[i] = sensor.getPressure(ADC_256);
  }
  for (int i = 0; i < (10 - 1); i++) {
    for (int o = 0; o < (10 - (i + 1)); o++) {
      if (pressureReadings[o] > pressureReadings[o + 1]) {
        int t = pressureReadings[o];
        pressureReadings[o] = pressureReadings[o + 1];
        pressureReadings[o + 1] = t;
      }
    }
  }

  pressure_abs = pressureReadings[4];

  // Taking our baseline pressure at the beginning we can find an approximate
  // change in altitude based on the differences in pressure.
  depth = altitude(pressure_abs, pressure_baseline);
}

// Given a pressure measurement P (mbar) and the pressure at a baseline P0 (mbar),
// return altitude (meters) above baseline.
double altitude(double P, double P0) {
  return (44330.0 * (1 - pow(P / P0, 1 / 5.255)));
}

// Function for updating yaw, pitch, roll
void updateYPR() {
  Vector mag = compass.readNormalize();
  yaw = atan2(mag.YAxis, mag.XAxis);
  yaw += declinationAngle;
  if (yaw < 0) {
    yaw += 2 * PI;
  }
  if (yaw > 2 * PI) {
    yaw -= 2 * PI;
  }
  yaw = yaw * 180 / M_PI;

  // Reading accelerometer values from IMU; calculating & filtering (median filter) pitch & roll
  double fpitcharray[5];
  double frollarray[5];
  for (int i = 0; i < 5; i++) {
    Vector accl = accelerometer.readNormalize();
    Vector faccl = accelerometer.lowPassFilter(accl, 0.5);
    fpitcharray[i] = -(atan2(faccl.XAxis, sqrt(faccl.YAxis  * faccl.YAxis + faccl.ZAxis * faccl.ZAxis)) * 180.0) / M_PI;
    frollarray[i] = (atan2(faccl.YAxis, faccl.ZAxis) * 180.0) / M_PI;
  }
  for (int i = 0; i < (5 - 1); i++) {
    for (int o = 0; o < (5 - (i + 1)); o++) {
      if (fpitcharray[o] > fpitcharray[o + 1]) {
        int t = fpitcharray[o];
        fpitcharray[o] = fpitcharray[o + 1];
        fpitcharray[o + 1] = t;
      }
    }
  }
  for (int i = 0; i < (5 - 1); i++) {
    for (int o = 0; o < (5 - (i + 1)); o++) {
      if (frollarray[o] > frollarray[o + 1]) {
        int t = frollarray[o];
        frollarray[o] = frollarray[o + 1];
        frollarray[o + 1] = t;
      }
    }
  }

  // Swap pitch & roll because our electronic engineers are very stupid...
  roll = fpitcharray[2];
  pitch = frollarray[2];
}

// Function to select right multiplexor channel
void selectMuxChannel() {
  if (muxChannel == 0) {
    digitalWrite(MULTIPLEXOR_PIN, LOW);
  }
  else if (muxChannel == 1) {
    digitalWrite(MULTIPLEXOR_PIN, HIGH);
  }
}

// Function to switch off/on LED
void switchLED() {
  if (isLED) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// Loop function
void loop() {
  updateYPR();
  updateDepth();
  if (receiveMessage() == 1) {
    sendReply();
  }
  controlPeripherals();
}

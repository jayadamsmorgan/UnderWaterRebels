#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>
#include <SparkFun_MS5803_I2C.h>
#include <ADXL345.h>
#include <HMC5883L.h>

#define MOTOR1PIN                5    // Some pin
#define MOTOR2PIN                6    // Some pin
#define MOTOR3PIN                3    // Some pin
#define MOTOR4PIN                9    // Some pin
#define MOTOR5PIN                11   // Some pin
#define MOTOR6PIN                12   // Some pin

#define MOTORLOWMICROSECONDS     1465
#define MOTORHIGHMICROSECONDS    1510
#define MOTORRANGE               400

#define MAIN_MANIP_ROT_PINA      13   // Some pin
#define MAIN_MANIP_ROT_PINB      14   // Some pin
#define MAIN_MANIP_ROT_PINPWM    15   // Some pin

#define MAIN_MANIP_TIGHT_PINA    16   // Some pin
#define MAIN_MANIP_TIGHT_PINB    17   // Some pin
#define MAIN_MANIP_TIGHT_PINPWM  18   // Some pin

#define SERVO_MANIPULATOR_PIN    19   // Some pin
#define SERVO_CAMERA_PIN         20   // Some pin

#define SERVO_UPDATE_WINDOW      30   // Delay for updating servo's angle

#define CAMERA_ANGLE_DELTA       3    // ?
#define MIN_CAMERA_ANGLE         0    // ?
#define MAX_CAMERA_ANGLE         160  // ?

#define BOTTOM_MANIP_ANGLE_DELTA 3    // ?
#define MAX_BOTTOM_MANIP_ANGLE   160  // ?
#define MIN_BOTTOM_MANIP_ANGLE   100  // ?

#define INCOMING_PACKET_SIZE     7
#define OUTCOMING_PACKET_SIZE    10

#define PITCH_KP                 2.0  // ?
#define PITCH_KI                 0.0  // ?
#define PITCH_KD                 0.0  // ?

#define DEPTH_KP                 2.0  // ?
#define DEPTH_KI                 0.0  // ?
#define DEPTH_KD                 0.0  // ?

#define YAW_KP                   0.0001  // ?
#define YAW_KI                   0.0  // ?
#define YAW_KD                   0.0  // ?

int MOTORMIDMICROSECONDS = (MOTORLOWMICROSECONDS + MOTORHIGHMICROSECONDS) / 2.0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 8, 177), remote_device;
char packetBuffer[INCOMING_PACKET_SIZE];
unsigned char replyBuffer[OUTCOMING_PACKET_SIZE];
EthernetUDP Udp;

ADXL345 accelerometer;
HMC5883L compass;
MS5803 sensor(ADDRESS_LOW);

Servo horMotor1, horMotor2, horMotor3, horMotor4;
Servo verMotor1, verMotor2;

Servo camera, bottomManip;
int camera_angle = (MAX_CAMERA_ANGLE + MIN_CAMERA_ANGLE) / 2, bottom_manip_angle = MAX_BOTTOM_MANIP_ANGLE;
unsigned long long prev_camera_servo_update, prev_manip_servo_update;

char servoCamDir = 0, manTightDir = 0, botManipDir = 0;

double declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
double yaw = 0, pitch = 0, roll = 0;
int depth = 0;

// Create variables to store results for depth calculations
double pressure_abs, pressure_baseline;

signed char js_val[5];
bool buttons[8];

bool isAutoDepth = false, isAutoPitch = false, isAutoYaw = false;
bool leak[8];

double speedK = 0.3;

double pitchSetpoint, pitchInput, pitchOutput;
PID autoPitchPID(&pitchInput, &pitchOutput, &pitchSetpoint, PITCH_KP, PITCH_KI, PITCH_KD, DIRECT);

double depthSetpoint, depthInput, depthOutput;
PID autoDepthPID(&depthInput, &depthOutput, &depthSetpoint, DEPTH_KP, DEPTH_KI, DEPTH_KD, DIRECT);

double yawSetpoint, yawInput, yawOutput;
PID autoYawPID(&yawInput, &yawOutput, &yawSetpoint, YAW_KP, YAW_KI, YAW_KD, DIRECT);

// Function for controlling motor system
void controlPeripherals() {
  // Auto modes realization:
  if (isAutoPitch && isAutoDepth) {
    Serial.println("Using AutoPitch & AutoDepth mode");
    autoPitchAndDepth();
  } else if (isAutoPitch) {
    Serial.println("Using AutoPitch mode");
    autoPitch();
    depthSetpoint = depth; // Set target for AutoDepth
  } else if (isAutoDepth) {
    Serial.println("Using AutoDepth mode");
    autoDepth();
  } else {
    // Set vertical thrust
    verticalMotorControl(verMotor1, js_val[2]);
    verticalMotorControl(verMotor2, js_val[2]);

    // Set target for AutoDepth
    depthSetpoint = depth;
  }

  // AutoYaw mode:
  if (isAutoYaw) {
    Serial.println("Using AutoYaw mode");
    autoYaw();
  } else {
    // Set horizontal thrust
    horizontalMotorControl(horMotor1, js_val[0], js_val[1], js_val[3]);
    horizontalMotorControl(horMotor2, js_val[0], js_val[1], js_val[3]);
    horizontalMotorControl(horMotor3, js_val[0], js_val[1], js_val[3]);
    horizontalMotorControl(horMotor4, js_val[0], js_val[1], js_val[3]);

    // Set target for AutoYaw
    yawSetpoint = yaw;
  }

  tightenManipulator(manTightDir);

  unsigned long long current_time = millis();
  if (servoCamDir != 0 && (current_time - prev_camera_servo_update >= SERVO_UPDATE_WINDOW)) {
    if (servoCamDir > 0) {
      camera_angle += CAMERA_ANGLE_DELTA;
      if (camera_angle > MAX_CAMERA_ANGLE)
        camera_angle = MAX_CAMERA_ANGLE;
    } else {
      camera_angle -= CAMERA_ANGLE_DELTA;
      if (camera_angle < MIN_CAMERA_ANGLE)
        camera_angle = MIN_CAMERA_ANGLE;
    }
  }
  camera.write(camera_angle);
  prev_camera_servo_update = millis();

  current_time = millis();
  if (botManipDir != 0 && (current_time - prev_manip_servo_update >= SERVO_UPDATE_WINDOW)) {
    if (botManipDir > 0) {
      bottom_manip_angle += BOTTOM_MANIP_ANGLE_DELTA;
      if (bottom_manip_angle > MAX_BOTTOM_MANIP_ANGLE)
        bottom_manip_angle = MAX_BOTTOM_MANIP_ANGLE;
    } else {
      bottom_manip_angle -= BOTTOM_MANIP_ANGLE_DELTA;
      if (bottom_manip_angle < MIN_BOTTOM_MANIP_ANGLE)
        bottom_manip_angle = MIN_BOTTOM_MANIP_ANGLE;
    }
  }
  bottomManip.write(bottom_manip_angle);
  prev_manip_servo_update = millis();

  rotateManipulator(js_val[4]);
}

// AutoPitch & AutoDepth mode
void autoPitchAndDepth() {
  // Preserving ROV from motor work on the surface
  if (depth == 0.0) {
    return;
  }

  depthInput = depth;
  pitchInput = rotationAngle(pitch, 0);

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

  verticalMotorControl(verMotor1, (char) output1);
  verticalMotorControl(verMotor2, (char) output2);
}

// AutoPitch mode
void autoPitch() {
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

  verticalMotorControl(verMotor1, (char) pitchOutput);
  verticalMotorControl(verMotor2, (char) - pitchOutput);
}

// AutoDepth mode
void autoDepth() {
  depthInput = depth;
  autoDepthPID.Compute();
  Serial.print("AutoDepth PID output is: "); Serial.println(depthOutput);
  Serial.print("Target depth is: ");         Serial.println(depthSetpoint);
  Serial.print("Current depth is: ");        Serial.println(depth);

  // Value correction:
  if (depthOutput > 100.0) {
    depthOutput = 100.0;
  }
  if (depthOutput < -100.0) {
    depthOutput = -100.0;
  }

  verticalMotorControl(verMotor1, (char) depthOutput);
  verticalMotorControl(verMotor2, (char) depthOutput);
}

// AutoYaw mode
void autoYaw() {
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
  if (abs(val) > 100) {
    return;
  }
  Serial.print("AutoYaw PID output is: "); Serial.println(val);
  Serial.print("Target yaw is: ");         Serial.println(yawSetpoint);
  Serial.print("Current yaw is: ");        Serial.println(yaw);

  horizontalMotorControl(horMotor1, 0, 0, val);
  horizontalMotorControl(horMotor2, 0, 0, -val);
  horizontalMotorControl(horMotor3, 0, 0, val);
  horizontalMotorControl(horMotor4, 0, 0, -val);
}

// Function for correct angles for PID
double rotationAngle(double currentAngle, double targetAngle) {
  double rotationAngle = currentAngle - targetAngle;
  if (rotationAngle >= 180.00) {
    rotationAngle = rotationAngle - 360.00;
  } else if (rotationAngle < -180.00) {
    rotationAngle = 360.0 - abs(rotationAngle);
  }
  return rotationAngle;
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
    for (int i = 0; i < 5; ++i)  {
      js_val[i] = (signed char)packetBuffer[i];
      Serial.print("js"); Serial.print(i); Serial.print(": "); Serial.println(js_val[i]);
    }
    for (int i = 0; i < 8; ++i) {
      buttons[i] = (packetBuffer[5] >> i) & 1;
      Serial.print("btn"); Serial.print(i); Serial.print(": "); Serial.println(buttons[i]);
    }

    if (buttons[0] == 1 && buttons[1] == 0) {
      servoCamDir = -1;
    } else if (buttons[0] == 0 && buttons[1] == 1) {
      servoCamDir = 1;
    } else {
      servoCamDir = 0;
    }

    if (buttons[2] == 1 && buttons[3] == 0) {
      manTightDir = -1;
    } else if (buttons[2] == 0 && buttons[3] == 1) {
      manTightDir = 1;
    } else {
      manTightDir = 0;
    }

    if (buttons[4] == 1 && buttons[5] == 0) {
      botManipDir = -1;
    } else if (buttons[4] == 0 && buttons[5] == 1) {
      botManipDir = 1;
    } else {
      botManipDir = 0;
    }

    char bit1 = (packetBuffer[6]) & 1;
    char bit2 = (packetBuffer[6] >> 1) & 1;
    char bit3 = (packetBuffer[6] >> 2) & 1;

    Serial.print("bit1: "); Serial.println(bit1);
    Serial.print("bit2: "); Serial.println(bit2);
    Serial.print("bit3: "); Serial.println(bit3);

    if (bit1 == 1) {
      speedK = 1.0;
    }
    if (bit2 == 1) {
      speedK = 0.6;
    }
    if (bit3 == 1) {
      speedK = 0.3;
    }

    isAutoDepth = (packetBuffer[6] >> 3) & 1;
    Serial.println(isAutoDepth);
    isAutoPitch = (packetBuffer[6] >> 4) & 1;
    Serial.println(isAutoPitch);
    isAutoYaw = (packetBuffer[6] >> 5) & 1;
    Serial.println(isAutoYaw);
    return 1;
  }
  else {
    //Serial.println("Size is incorrect.");
    return 0;
  }
}

// Forming & sending packet to PC via UDP
void sendReply() {
  Serial.print("PC is on :"); Serial.println(remote_device);

  Serial.println("Forming packet...");
  replyBuffer[0] = ((int) (yaw * 100.0) >> 8) & 0xFF;
  replyBuffer[1] = ((int) (yaw * 100.0)) & 0xFF;
  replyBuffer[2] = ((int) (pitch * 100.0) >> 8) & 0xFF;
  replyBuffer[3] = ((int) (pitch * 100.0)) & 0xFF;
  replyBuffer[4] = ((int) (roll * 100.0) >> 8) & 0xFF;
  replyBuffer[5] = ((int) (roll * 100.0)) & 0xFF;
  replyBuffer[6] = ((int) (depth * 100.0) >> 8) & 0xFF;
  replyBuffer[7] = ((int) (depth * 100.0)) & 0xFF;
  for (int i = 0; i < 8; ++i) {
    replyBuffer[8] |= leak[i] << i;
  }
  //replyBuffer[9] = ...

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
  if (sum > 100.0) sum = 100.0;
  if (sum < (-100.0)) sum = -100.0;
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
  short POW = 0;
  POW = short(abs(m) * 255.0 / 100.0);
  analogWrite(MAIN_MANIP_ROT_PINPWM, POW);
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
  analogWrite(MAIN_MANIP_TIGHT_PINPWM, 255);
}

void setup() {
  // Init I2C connection for IMU
  Wire.begin();
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

  // Init bottom manipulator & main camera
  camera.attach(SERVO_CAMERA_PIN);
  camera.write(camera_angle);
  bottomManip.attach(SERVO_MANIPULATOR_PIN);
  bottomManip.write(bottom_manip_angle);

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
    pressureReadings[i] = sensor.getPressure(ADC_4096);
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
  depth = altitude(pressure_abs , pressure_baseline);
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


  double fpitcharray[10];
  double frollarray[10];
  for (int i = 0; i < 10; i++) {
    Vector accl = accelerometer.readNormalize();
    Vector faccl = accelerometer.lowPassFilter(accl, 0.5);
    fpitcharray[i] = -(atan2(faccl.XAxis, sqrt(faccl.YAxis  * faccl.YAxis + faccl.ZAxis * faccl.ZAxis)) * 180.0) / M_PI;
    frollarray[i] = (atan2(faccl.YAxis, faccl.ZAxis) * 180.0) / M_PI;
  }


  for (int i = 0; i < (10 - 1); i++) {
    for (int o = 0; o < (10 - (i + 1)); o++) {
      if (fpitcharray[o] > fpitcharray[o + 1]) {
        int t = fpitcharray[o];
        fpitcharray[o] = fpitcharray[o + 1];
        fpitcharray[o + 1] = t;
      }
    }
  }
  for (int i = 0; i < (10 - 1); i++) {
    for (int o = 0; o < (10 - (i + 1)); o++) {
      if (frollarray[o] > frollarray[o + 1]) {
        int t = frollarray[o];
        frollarray[o] = frollarray[o + 1];
        frollarray[o + 1] = t;
      }
    }
  }

  pitch = fpitcharray[4];
  roll = frollarray[4];

  yaw += declinationAngle;

  if (yaw < 0) {
    yaw += 2 * PI;
  }
  if (yaw > 2 * PI) {
    yaw -= 2 * PI;
  }

  yaw = yaw * 180 / M_PI;
}

void loop() {
  updateYPR();
  //updateDepth();
  /*if (receiveMessage() == 1) {
    sendReply();
    }*/
  controlPeripherals();
}

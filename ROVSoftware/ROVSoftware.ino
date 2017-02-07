#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>
#include <SparkFun_MS5803_I2C.h>

#define MOTOR1PIN                5    // Some pin
#define MOTOR2PIN                6    // Some pin
#define MOTOR3PIN                3    // Some pin
#define MOTOR4PIN                9    // Some pin
#define MOTOR5PIN                11   // Some pin
#define MOTOR6PIN                12   // Some pin

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
#define PITCH_KI                 1.0  // ?
#define PITCH_KD                 0.5  // ?

#define DEPTH_KP                 2.0  // ?
#define DEPTH_KI                 1.0  // ?
#define DEPTH_KD                 0.5  // ?

#define YAW_KP                   2    // ?
#define YAW_KI                   1    // ?
#define YAW_KD                   0.5  // ?

#define MOTOR1LOW                1452 // ?
#define MOTOR1HIGH               1568 // ?
#define MOTOR1RANGE              354  // ?

#define MOTOR2LOW                1546 // ?
#define MOTOR2HIGH               1660 // ?
#define MOTOR2RANGE              384  // ?

#define MOTOR3LOW                1590 // ?
#define MOTOR3HIGH               1694 // ?
#define MOTOR3RANGE              474  // ?

#define MOTOR4LOW                1452 // ?
#define MOTOR4HIGH               1568 // ?
#define MOTOR4RANGE              354  // ?

#define MOTOR5LOW                1452 // ?
#define MOTOR5HIGH               1568 // ?
#define MOTOR5RANGE              354  // ?

#define MOTOR6LOW                1452 // ?
#define MOTOR6HIGH               1568 // ?
#define MOTOR6RANGE              354  // ?

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 242), remote_device;
char packetBuffer[INCOMING_PACKET_SIZE];
unsigned char replyBuffer[OUTCOMING_PACKET_SIZE];
EthernetUDP Udp;
  
Servo horMotor1, horMotor2, horMotor3, horMotor4;
Servo verMotor1, verMotor2;

Servo camera, bottomManip;
int camera_angle = (MAX_CAMERA_ANGLE + MIN_CAMERA_ANGLE) / 2, bottom_manip_angle = MAX_BOTTOM_MANIP_ANGLE;
unsigned long long prev_camera_servo_update, prev_manip_servo_update;

char servoCamDir = 0, manTightDir = 0, botManipDir = 0;

float yaw = 0, pitch = 0, roll = 0;
int depth = 0;

MS5803 sensor(ADDRESS_LOW);

// Create variables to store results for depth calculations
double pressure_abs, pressure_baseline;

signed char js_val[5];
bool buttons[8];

bool isAutoDepth = false, isAutoPitch = false, isAutoYaw = false;
bool leak[8];

float speedK = 1;

double pitchSetpoint, pitchInput, pitchOutput;
PID autoPitchPID(&pitchInput, &pitchOutput, &pitchSetpoint, PITCH_KP, PITCH_KI, PITCH_KD, DIRECT);

double depthSetpoint, depthInput, depthOutput;
PID autoDepthPID(&depthInput, &depthOutput, &depthSetpoint, DEPTH_KP, DEPTH_KI, DEPTH_KD, DIRECT);

double yawSetpoint, yawInput, yawOutput;
PID autoYawPID(&yawInput, &yawOutput, &yawSetpoint, YAW_KP, YAW_KI, YAW_KD, DIRECT);

// Function for controlling motor system
void controlPeripherals() {
  // Auto modes realization:
  if ((isAutoPitch && isAutoDepth) || js_val[2] == 0) {
    Serial.println("Using AutoPitch & AutoDepth mode");
    autoPitchAndDepth();
  } else if (isAutoPitch) {
    Serial.println("Using AutoPitch mode");
    autoPitch();
    depthSetpoint = depth; // Set target for AutoDepth
  } else if (isAutoDepth) {
    Serial.println("Using AutoDepth mode");
    autoDepth();
    pitchSetpoint = pitch; // Set target for AutoPitch
  } else {
    // Set vertical thrust
    verticalMotorControl(verMotor1, js_val[2], MOTOR5LOW, MOTOR5HIGH, MOTOR5RANGE);
    verticalMotorControl(verMotor2, js_val[2], MOTOR6LOW, MOTOR6HIGH, MOTOR6RANGE);
    
    // Set targets for AutoPitch & AutoDepth
    pitchSetpoint = pitch;
    depthSetpoint = depth;
  }
  
  // AutoYaw mode:
  if (isAutoYaw) {
    Serial.println("Using AutoYaw mode");
    autoYaw();
  } else {
    // Set horizontal thrust
    horizontalMotorControl(horMotor1, js_val[0], js_val[1], js_val[3], MOTOR1LOW, MOTOR1HIGH, MOTOR1RANGE);
    horizontalMotorControl(horMotor2, js_val[0], js_val[1], js_val[3], MOTOR2LOW, MOTOR2HIGH, MOTOR2RANGE);
    horizontalMotorControl(horMotor3, js_val[0], js_val[1], js_val[3], MOTOR3LOW, MOTOR3HIGH, MOTOR3RANGE);
    horizontalMotorControl(horMotor4, js_val[0], js_val[1], js_val[3], MOTOR4LOW, MOTOR4HIGH, MOTOR4RANGE);
    
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
  pitchInput = rotationAngle(pitch, pitchSetpoint);
  
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
  
  verticalMotorControl(verMotor1, (char) output1, MOTOR5LOW, MOTOR5HIGH, MOTOR5RANGE);
  verticalMotorControl(verMotor2, (char) output2, MOTOR6LOW, MOTOR6HIGH, MOTOR6RANGE);
}

// AutoPitch mode
void autoPitch() {
  pitchInput = rotationAngle(pitch, pitchSetpoint);
  autoPitchPID.Compute();
  Serial.print("AutoPitch PID output is: "); Serial.println(pitchOutput);
  Serial.print("Target pitch is: ");         Serial.println(pitchSetpoint);
  Serial.print("Current pitch is: ");        Serial.println(pitch);
  
  // Value correction:
  if (pitchOutput > 100.0) {
    pitchOutput = 100.0;
  }
  if (pitchOutput < -100.0) {
    pitchOutput = -100.0;
  }
  
  verticalMotorControl(verMotor1, (char) pitchOutput, MOTOR5LOW, MOTOR5HIGH, MOTOR5RANGE);
  verticalMotorControl(verMotor2, (char) -pitchOutput, MOTOR6LOW, MOTOR6HIGH, MOTOR6RANGE);
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
  
  verticalMotorControl(verMotor1, (char) depthOutput, MOTOR5LOW, MOTOR5HIGH, MOTOR5RANGE);
  verticalMotorControl(verMotor2, (char) depthOutput, MOTOR6LOW, MOTOR6HIGH, MOTOR6RANGE);
}

// AutoYaw mode
void autoYaw() {
  yawInput = rotationAngle(yaw, yawSetpoint);
  autoYawPID.Compute();
  horizontalMotorControl(horMotor1, 0, 0, yawOutput, MOTOR1LOW, MOTOR1HIGH, MOTOR1RANGE);
  horizontalMotorControl(horMotor2, 0, 0, yawOutput, MOTOR2LOW, MOTOR2HIGH, MOTOR2RANGE);
  horizontalMotorControl(horMotor3, 0, 0, yawOutput, MOTOR3LOW, MOTOR3HIGH, MOTOR3RANGE);
  horizontalMotorControl(horMotor4, 0, 0, yawOutput, MOTOR4LOW, MOTOR4HIGH, MOTOR4RANGE);
}

// Function for correct angles for PID
double rotationAngle(double currentAngle, double targetAngle) {
  double rotationAngle = yawSetpoint - yaw;
  if (abs(rotationAngle) > 180.0) {
    if (rotationAngle < 0) {
      rotationAngle = 360.0 - abs(rotationAngle);
    } else if (rotationAngle > 0) {
      rotationAngle = abs(rotationAngle) - 360.0;
    }
  }
}

// Receiving messages from PC & parsing
char receiveMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    Serial.print("Received packet of size: ");
    Serial.print(packetSize);
  }
  if(packetSize == INCOMING_PACKET_SIZE) {
    Serial.println(". Size is correct.");
    remote_device = Udp.remoteIP();
    Udp.read(packetBuffer, INCOMING_PACKET_SIZE);
    for (int i = 0; i < 5; ++i)
      js_val[i] = (signed char)packetBuffer[i];
    for (int i = 0; i < 8; ++i) {
      buttons[i] = (packetBuffer[5] >> i) & 1;
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
    isAutoPitch = (packetBuffer[6] >> 4) & 1;
    isAutoYaw = (packetBuffer[6] >> 5) & 1;
    return 1;
  }
  else {
    Serial.println(". Size is incorrect.");
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
void horizontalMotorControl(Servo motor, char x, char y, char z, int motorLow, int motorHigh, int motorRange) {
  short POW = 0;
  float sum = x + y + z;
  if(sum > 100.0) sum = 100.0;
  if(sum < (-100.0)) sum = -100.0;
  Serial.print("Horizontal motor pow: "); Serial.println(POW);
  POW = short((sum * (motorRange / 100.0)) * speedK);
  if (POW == 0) {
    return;
  }
  if (POW < 0) {
    motor.writeMicroseconds(motorLow + POW);
  }
  if (POW > 0) {
    motor.writeMicroseconds(motorHigh + POW);
  }
}

// Function to control vertical brushless motors
void verticalMotorControl(Servo motor, short z, int motorLow, int motorHigh, int motorRange) {
  short POW = 0;
  float sum = z;
  if(sum > 100.0) sum = 100.0;
  if(sum < (-100.0)) sum = -100.0;
  Serial.print("Vertical motor pow: "); Serial.println(POW);
  POW = short((sum * (motorRange / 100.0)) * speedK);
  if (POW == 0) {
    return;
  }
  if (POW < 0) {
    motor.writeMicroseconds(motorLow + POW);
  }
  if (POW > 0) {
    motor.writeMicroseconds(motorHigh + POW);
  }
}

// Function to rotate manipulator
void rotateManipulator(short m) {
  if (m > 0){
    digitalWrite(MAIN_MANIP_ROT_PINA, HIGH);
    digitalWrite(MAIN_MANIP_ROT_PINB, LOW);
  }
  if (m < 0){
    digitalWrite(MAIN_MANIP_ROT_PINA, LOW);
    digitalWrite(MAIN_MANIP_ROT_PINB, HIGH);
  }
  if (m == 0){
    digitalWrite(MAIN_MANIP_ROT_PINA, LOW);
    digitalWrite(MAIN_MANIP_ROT_PINB, LOW);
  }
  short POW = 0;
  POW = short(abs(m) * 255.0 / 100.0);
  analogWrite(MAIN_MANIP_ROT_PINPWM, POW);
}

// Function to tight manipulator
void tightenManipulator(char dir) {
  if (dir > 0){
    digitalWrite(MAIN_MANIP_TIGHT_PINA, HIGH);
    digitalWrite(MAIN_MANIP_TIGHT_PINB, LOW);
  }
  if (dir < 0){
    digitalWrite(MAIN_MANIP_TIGHT_PINA, LOW);
    digitalWrite(MAIN_MANIP_TIGHT_PINB, HIGH);
  }
  if (dir == 0){
    digitalWrite(MAIN_MANIP_TIGHT_PINA, LOW);
    digitalWrite(MAIN_MANIP_TIGHT_PINB, LOW);
  }
  analogWrite(MAIN_MANIP_TIGHT_PINPWM, 255);
}

void setup() {
  // Init I2C connection for IMU
  Wire.begin();

  // Init brushless motors
  delay(1000);
  horMotor1.attach(MOTOR1PIN);
  horMotor2.attach(MOTOR2PIN);
  horMotor3.attach(MOTOR3PIN);
  horMotor4.attach(MOTOR4PIN);
  verMotor1.attach(MOTOR5PIN);
  verMotor2.attach(MOTOR6PIN);
  horMotor1.write(1600);
  horMotor2.write(1600);
  horMotor3.write(1600);
  horMotor4.write(1600);
  verMotor1.write(1600);
  verMotor2.write(1600);
  
  // Ethernet & Serial port init
  Ethernet.begin(mac,ip);
  Udp.begin(8000);
  Serial.begin(250000);
  
  // Init bottom manipulator & main camera
  camera.attach(SERVO_CAMERA_PIN);
  camera.write(camera_angle);
  bottomManip.attach(SERVO_MANIPULATOR_PIN);
  bottomManip.write(bottom_manip_angle);
  
  // Init PID settings
  autoPitchPID.SetMode(AUTOMATIC);
  autoDepthPID.SetMode(AUTOMATIC);
  autoYawPID.SetMode(AUTOMATIC);
  
  // Some delay for motors...
  delay(1000);
  // Retrieve calibration constants for conversion math.
  sensor.reset();
  sensor.begin();  
  pressure_baseline = sensor.getPressure(ADC_4096);
}

// Function for updating yaw, pitch, roll
void updateYPR() {
  // TODO yaw, pitch, roll update
}

// Function for updating depth
void updateDepth() {
  // Read pressure from the sensor in mbar.
  pressure_abs = sensor.getPressure(ADC_4096);

  // Taking our baseline pressure at the beginning we can find an approximate
  // change in altitude based on the differences in pressure.   
  depth = altitude(pressure_abs , pressure_baseline);
}

// Given a pressure measurement P (mbar) and the pressure at a baseline P0 (mbar),
// return altitude (meters) above baseline.
double altitude(double P, double P0) {
  return(44330.0*(1-pow(P/P0,1/5.255)));
}
  

void loop() {
  updateYPR();
  updateDepth();
  if (receiveMessage() == 1) {
    sendReply();
  }
  controlPeripherals();
}

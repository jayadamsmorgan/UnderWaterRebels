#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>

#define MOTOR1PIN 7 // Some pin
#define MOTOR2PIN 8 // Some pin
#define MOTOR3PIN 9 // Some pin
#define MOTOR4PIN 10 // Some pin
#define MOTOR5PIN 11 // Some pin
#define MOTOR6PIN 12 // Some pin

#define SERVO_MANIPULATOR_PIN 13 // Some pin
#define SERVO_CAMERA_PIN      14 // Some pin

#define SERVO_UPDATE_WINDOW           30 // Delay for updating servo's angle
#define DEPTH_AND_PITCH_UPDATE_WINDOW 250 // Delay for switching autoDepth & autoPitch

#define CAMERA_ANGLE_DELTA 3 // ?
#define MIN_CAMERA_ANGLE   0 // ?
#define MAX_CAMERA_ANGLE   160 // ?

#define BOTTOM_MANIP_ANGLE_DELTA 3 // ?
#define MAX_BOTTOM_MANIP_ANGLE   160 // ?
#define MIN_BOTTOM_MANIP_ANGLE   100 // ?

#define INCOMING_PACKET_SIZE  9
#define OUTCOMING_PACKET_SIZE 10

#define PITCH_KP 2.0 // ?
#define PITCH_KI 1.0 // ?
#define PITCH_KD 0.5 // ?

#define DEPTH_KP 2.0 // ?
#define DEPTH_KI 1.0 // ?
#define DEPTH_KD 0.5 // ?

#define YAW_KP   2 // ?
#define YAW_KI   1 // ?
#define YAW_KD   0.5 // ?

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 242), remote_device;
char packetBuffer[INCOMING_PACKET_SIZE]; // Buffer to hold incoming packet,
unsigned char replyBuffer[OUTCOMING_PACKET_SIZE]; // A string to send back
EthernetUDP Udp;
  
Servo horMotor1, horMotor2, horMotor3, horMotor4;
Servo verMotor1, verMotor2;

Servo camera, bottomManip;
int camera_angle = (MAX_CAMERA_ANGLE + MIN_CAMERA_ANGLE) / 2, bottom_manip_angle = MAX_BOTTOM_MANIP_ANGLE;
unsigned long long prev_camera_servo_update, prev_manip_servo_update;

float yaw = 0, pitch = 0, roll = 0;
int depth = 0;
  
short min_speed = 0, max_speed = 400;

signed char js_val[5];
bool buttons[7];

bool isAutoDepth = false, isAutoPitch = false, isAutoYaw = false;

unsigned char depth_and_pitch_update = 0;

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
    depth_and_pitch_update++;
    // Enabling and disabling AutoPitch & AutoDepth alternately for correct work
    if (depth_and_pitch_update <= DEPTH_AND_PITCH_UPDATE_WINDOW) 
      autoPitch();
    else if (depth_and_pitch_update > DEPTH_AND_PITCH_UPDATE_WINDOW)
      autoDepth();
    if (depth_and_pitch_update >= 2 * DEPTH_AND_PITCH_UPDATE_WINDOW)
      depth_and_pitch_update = 0;
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
    verticalMotorControl(verMotor1, js_val[2]);
    verticalMotorControl(verMotor2, js_val[2]);
    
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
    horizontalMotorControl(horMotor1, js_val[1], js_val[0], js_val[3]);
    horizontalMotorControl(horMotor2, js_val[1], js_val[0], js_val[3]);
    horizontalMotorControl(horMotor3, js_val[1], js_val[0], js_val[3]);
    horizontalMotorControl(horMotor4, js_val[1], js_val[0], js_val[3]);
    
    // Set target for AutoYaw
    yawSetpoint = yaw;
  }
}

// AutoPitch mode
void autoPitch() {
  pitchInput = pitch;
  autoPitchPID.Compute();
  Serial.print("AutoPitch PID output is: "); Serial.println(pitchOutput);
  Serial.print("Target pitch is: ");         Serial.println(pitchSetpoint);
  Serial.print("Current pitch is: ");        Serial.println(pitch);
  verticalMotorControl(verMotor1, (char) pitchOutput);
  verticalMotorControl(verMotor2, (char) -pitchOutput);
}

// AutoDepth mode
void autoDepth() {
  depthInput = depth;
  autoDepthPID.Compute();
  Serial.print("AutoDepth PID output is: "); Serial.println(depthOutput);
  Serial.print("Target depth is: ");         Serial.println(depthSetpoint);
  Serial.print("Current depth is: ");        Serial.println(depth);
  verticalMotorControl(verMotor1, (char) depthOutput);
  verticalMotorControl(verMotor2, (char) depthOutput);
}

// AutoYaw mode
void autoYaw() {
  double rotationAngle = yawSetpoint - yaw;
  if (abs(rotationAngle) > 180.0) {
    if (rotationAngle < 0) {
      rotationAngle = 360 - abs(rotationAngle);
    } else if (rotationAngle > 0) {
      rotationAngle = abs(rotationAngle) - 360;
    }
  }
  yawInput = rotationAngle;
  autoYawPID.Compute();
  horizontalMotorControl(horMotor1, 0, 0, yawOutput);
  horizontalMotorControl(horMotor2, 0, 0, yawOutput);
  horizontalMotorControl(horMotor3, 0, 0, yawOutput);
  horizontalMotorControl(horMotor4, 0, 0, yawOutput);
}

// Receiving messages from PC & parsing
char receiveMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0){
    Serial.print("Received packet of size: ");
    Serial.println(packetSize);
  }
  if(packetSize == INCOMING_PACKET_SIZE) {
    Serial.println("Received packet with correct size");
    remote_device = Udp.remoteIP();
    Udp.read(packetBuffer , INCOMING_PACKET_SIZE);
    for (int i = 0; i < 5; ++i)
      js_val[i] = (signed char)packetBuffer[i];
    min_speed = (unsigned char)packetBuffer[5];
    max_speed = (unsigned char)packetBuffer[6];
    isAutoDepth = packetBuffer[8] & 1;
    isAutoPitch = (packetBuffer[8] >> 1) & 1;
    isAutoYaw = (packetBuffer[8] >> 2) & 1;
    for (int i = 0; i < 7; ++i){
      buttons[i] = (packetBuffer[7] >> i) & 1;
    }
    return 1;
  }
  else
    return 0;
}

// Forming & sending packet to PC via UDP
void sendReply() {
  Serial.print("PC is on :"); Serial.println(remote_device);
  replyBuffer[0] = (depth >> 8) & 0xFF;
  replyBuffer[1] = depth & 0xFF;
  yaw *= 100;
  pitch *= 100;
  roll *= 100;
  replyBuffer[2] = ((int)yaw >> 8) & 0xFF;
  replyBuffer[3] = ((int)yaw) & 0xFF;
  replyBuffer[4] = ((int)pitch >> 8) & 0xFF;
  replyBuffer[5] = ((int)pitch) & 0xFF;
  replyBuffer[6] = ((int)roll >> 8) & 0xFF;
  replyBuffer[7] = ((int)roll) & 0xFF;
  
  Serial.println("Replying...");
  Udp.beginPacket(remote_device, Udp.remotePort());
  Serial.println("Forming packet");
  Serial.println(Udp.write(replyBuffer, OUTCOMING_PACKET_SIZE));
  Serial.println("Writing packet");
  Udp.endPacket();
  Serial.println("Endpacket");
  return;
}
    
// Function to control horizontal brushless motors
void horizontalMotorControl(Servo motor, char x, char y, char z) {
  short POW = 0;
  float sum = x + y + z;
  if(sum > 100) sum = 100;
  if(sum < (-100)) sum = -100;
  Serial.print("Horizontal motor pow: "); Serial.println(POW);
  POW = short(sum * (float(max_speed - min_speed) / 100));
  motor.writeMicroseconds(1460 + POW);
}

// Function to control vertical brushless motors
void verticalMotorControl(Servo motor, short z) {
  short POW = 0;
  float sum = z;
  if(sum > 100) sum = 100;
  if(sum < (-100)) sum = -100;
  Serial.print("Vertical motor pow: "); Serial.println(POW);
  POW = short(sum * (float(max_speed - min_speed) / 100));
  motor.writeMicroseconds(1460 + POW);
}

// Function to rotate manipulator
void rotateManipulator(char PinA, char PinB, char dir) {
  if (dir > 0){
    digitalWrite(PinA, HIGH);
    digitalWrite(PinB, LOW);
  }
  if (dir < 0){
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, HIGH);
  }
  if (dir == 0){
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, LOW);
  }
}

// Function to tight manipulator
void tightenManipulator(char PinA, char PinB, char dir) {
  if (dir > 0){
    digitalWrite(PinA, HIGH);
    digitalWrite(PinB, LOW);
  }
  if (dir < 0){
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, HIGH);
  }
  if (dir == 0){
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, LOW);
  }
}

void setup() {
  // Init I2C connection for IMU
  Wire.begin();
  
  // Init brushless motors
  horMotor1.attach(MOTOR1PIN);
  horMotor2.attach(MOTOR2PIN);
  horMotor3.attach(MOTOR3PIN);
  horMotor4.attach(MOTOR4PIN);
  verMotor1.attach(MOTOR5PIN);
  verMotor2.attach(MOTOR6PIN);
  horMotor1.write(90);
  horMotor2.write(90);
  horMotor3.write(90);
  horMotor4.write(90);
  verMotor1.write(90);
  verMotor2.write(90);
  
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
  delay(1500);
}

void loop() {
  
}

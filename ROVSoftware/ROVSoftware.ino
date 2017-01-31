#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>

#define MOTOR1PIN 7 //some pin
#define MOTOR2PIN 8 //some pin
#define MOTOR3PIN 9 //some pin
#define MOTOR4PIN 10 //some pin
#define MOTOR5PIN 11 //some pin
#define MOTOR6PIN 12 //some pin

#define SERVO_MANIPULATOR_PIN 13 //some pin
#define SERVO_CAMERA_PIN 14 //some pin

#define SERVO_UPDATE_WINDOW 30 // Delay for updating servo's angle
#define DEPTH_AND_PITCH_UPDATE_WINDOW 250 // Delay for switching autoDepth & autoPitch

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,111);
  
Servo horMotor1, horMotor2, horMotor3, horMotor4;
Servo verMotor1, verMotor2;

float yaw = 0, pitch = 0, roll = 0;

short min_speed = 0, max_speed = 255;
signed char js_val[5];
bool isAutoDepth = false, isAutoPitch = false, isAutoYaw = false;

int depth_and_pitch_update = 0;

// Function for controlling motor system
void controlPeripherals() {
  // Auto modes realization:
  
  if (isAutoYaw) {
    autoYaw();
  }
  if ((isAutoPitch && isAutoYaw) || js_val[2] == 0) {
    depth_and_pitch_update++;
    if (depth_and_pitch_update <= DEPTH_AND_PITCH_UPDATE_WINDOW)
      autoPitch();
    else if (depth_and_pitch_update <= DEPTH_AND_PITCH_UPDATE_WINDOW)
      autoDepth();
    if (depth_and_pitch_update >= 2 * DEPTH_AND_PITCH_UPDATE_WINDOW)
      depth_and_pitch_update = 0;
  } else if (isAutoPitch) {
    autoPitch();
  } else if (isAutoDepth) {
    autoDepth();
  }
  
  if (!(isAutoDepth && isAutoPitch) && js_val[2] != 0) {
    
  }
}

// AutoPitch mode
void autoPitch() {
  float value = pitchPID();
  verticalMotorControl(verMotor1, value);
  verticalMotorControl(verMotor2, -value);
}

// AutoDepth mode
void autoDepth() {
  float value = depthPID();
  verticalMotorControl(verMotor1, value);
  verticalMotorControl(verMotor2, value);
}

// Calculation PID for yaw
void autoYaw() {
  horizontalMotorControl(
}

// Calculation PID for pitch    
char pitchPID() {
  //TODO calculation of pitchPID
}

// Calculation PID for depth
char depthPID() {
  //TODO calculation of depthPID
}
    
// Function to control horizontal brushless motors
void horizontalMotorControl(Servo motor, char x, char y, char z) {
  short POW = 0;
  float sum = x + y + z;
  if(sum > 100) sum = 100;
  if(sum < (-100)) sum = -100;
  Serial.print("Horizontal motor pow: "); Serial.println(POW);
  POW = map(POW, -100, 100, 1060, 1860);
  motor.writeMicroseconds(POW);
}

// Function to control vertical brushless motors
void verticalMotorControl(Servo motor, short z) {
  short POW = 0;
  float sum = z;
  if(sum > 100) sum = 100;
  if(sum < (-100)) sum = -100;
  Serial.print("Vertical motor pow: "); Serial.println(POW);
  POW = map(POW, -100, 100, 1060, 1860);
  motor.writeMicroseconds(POW);
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
  
  // Some delay for motors...
  delay(2000);
}

void loop() {
  
}

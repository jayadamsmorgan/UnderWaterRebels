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

#define SERVO

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,111);
  
Servo horMotor1, horMotor2, horMotor3, horMotor4;
Servo verMotor1, verMotor2;

float yaw = 0, pitch = 0, roll = 0;

short min_speed = 0, max_speed = 255;
signed char js_val[5];
bool isAutoDepth = false, isAutoPitch = false, isAutoYaw = false;

void controlPeripherals() {
  if (isAutoYaw) {
    autoYaw();
  }
  if (isAutoPitch && isAutoYaw) {
    autoPitchYaw();
  } else if (isAutoPitch) {
    autoPitch();
  } else if (isAutoDepth) {
    autoDepth();
  }
}

void autoPitchYaw() {
  
}

void autoPitch() {
  float value = pitchPID();
  verticalMotorControl(verMotor1, value);
  verticalMotorControl(verMotor2, -value);
}

void autoDepth() {
  float value = depthPID();
  verticalMotorControl(verMotor1, value);
  verticalMotorControl(verMotor2, value);
}

void autoYaw() {
  horizontalMotorControl(
}

char pitchPID() {
  //TODO calculation of pitchPID
}

char depthPID() {
  //TODO calculation of depthPID
}

char pitchAndDepthPID() {
  //TODO calculation of pitchAndDepthPID
}

void horizontalMotorControl(Servo motor, char x, char y, char z) {
  short POW = 0;
  float sum = x + y + z;
  if(sum > 100) sum = 100;
  if(sum < (-100)) sum = -100;
  POW = short(sum * (float(max_speed - min_speed) / 100));
  Serial.print("Horizontal motor pow: "); Serial.println(POW);
  POW = map(min_speed + POW, -255, 255, 0, 180);
  motor.writeMicroseconds(POW);
}

void verticalMotorControl(Servo motor, short z) {
  short POW = 0;
  float sum = z;
  if(sum > 100) sum = 100;
  if(sum < (-100)) sum = -100;
  POW = short(sum * (float(max_speed - min_speed) / 100));
  Serial.print("Vertical motor pow: "); Serial.println(POW);
  POW = map(min_speed + POW, -255, 255, 0, 180);
  motor.writeMicroseconds(POW);
}

void setup() {
  Wire.begin();
  
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
  delay(2000);

  
}

void loop() {
  
}

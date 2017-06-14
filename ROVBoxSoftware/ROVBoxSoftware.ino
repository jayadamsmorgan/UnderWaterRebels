#include <Wire.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>

// Writing bytes to the lcd on different arduinos
#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define pinA          A2 // Ammeter pin
#define pinV          A1 // Voltmeter voltage divider pin
#define pinPWM_OUT     5 // Fan's pwm writing rpm pin
#define pinTemp       A0 // Temperature sensor pin

#define R1 100000 // Real resistance on first resistor in voltage divider
#define R2 10000  // Real resistance on second resistor in voltage divider

#define k 5.0 // Real Voltage between GND & 5V on arduino

#define MAX_TEMP 35 // Maximum critical temperature in box

OneWire ds(pinTemp); // One-Wire BS18B20 temperature
LiquidCrystal_I2C lcd(0x3F, 20, 4); // I2C 2004 LCD

long long prev_time_alarm;
long long prev_time_update;
boolean isBacklight;

// Some variables for getting fan's rpm
const int numreadings = 10;
int readings[numreadings];
unsigned long average = 0;
int index = 0;
unsigned long total;
volatile int rpmcount = 0;
unsigned long rpm = 0;
unsigned long lastmillis = 0;

void setup() {
  // Serial init
  Serial.begin(115200);

  // Pins init
  pinMode(pinA, INPUT_PULLUP);
  digitalWrite(pinA, HIGH);
  pinMode(pinV, INPUT);
  pinMode(pinPWM_OUT, OUTPUT);
  attachInterrupt(0, rpm_fan, FALLING);
  digitalWrite(3, HIGH);

  // Display init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(10, 3);
  lcd.print("LOADING...");
  delay(2000);
  lcd.clear();
  lcd.noBacklight();
  lcd.setCursor(8, 0);
  lcd.print("CURRENT:");
  lcd.setCursor(19, 0);
  lcd.print("A");
  lcd.setCursor(8, 1);
  lcd.print("VOLTAGE:");
  lcd.setCursor(19, 1);
  lcd.print("V");
  lcd.setCursor(0, 2);
  lcd.print("RPM:");
  lcd.setCursor(9, 2);
  lcd.print("RPM");
  lcd.setCursor(0, 3);
  lcd.print("TEMPERATURE:");
  lcd.setCursor(15, 3);
  lcd.printByte(223);
  lcd.setCursor(16, 3);
  lcd.print("C");
}

void alarm() {
  // Turning off/on one time in 300ms
  if (millis() - prev_time_alarm > 300) {
    if (isBacklight) {
      lcd.noBacklight();
      isBacklight = false;
    } else {
      lcd.backlight();
      isBacklight = true;
    }
    prev_time_alarm = millis();
  }
}

int updateCurrent() {
  int currentArray[10];
  for (int i = 0; i < 10; i++) {
    currentArray[i] = analogRead(pinA);
  }
  for (int i = 0; i < (10 - 1); i++) {
    for (int o = 0; o < (10 - (i + 1)); o++) {
      if (currentArray[o] > currentArray[o + 1]) {
        int t = currentArray[o];
        currentArray[o] = currentArray[o + 1];
        currentArray[o + 1] = t;
      }
    }
  }
  float value = -(30.9/365) * currentArray[4] + (15975.3/365);
  int returnableValue;
  if (value - (int) value >= 0.5f) {
    returnableValue = (int) value + 1;
  } else {
    returnableValue = (int) value;
  }
  value += 3;
  return value;
}

void updateValues(int current, int voltage, int rpm, int temperature) {
  // Printing current based on length
  lcd.setCursor(16, 0);
  if (current < 10) {
    lcd.print("  ");
    lcd.setCursor(18, 0);
  } else if (current < 100) {
    lcd.print(" ");
    lcd.setCursor(17, 0);
  }
  lcd.print(current);

  // Printing voltage based on length
  lcd.setCursor(16, 1);
  if (voltage < 10) {
    lcd.print("  ");
    lcd.setCursor(18, 1);
  } else if (voltage < 100) {
    lcd.print(" ");
    lcd.setCursor(17, 1);
  }
  lcd.print(voltage);

  // Printing RPM based
  lcd.setCursor(4, 2);
  if (rpm < 10) {
    lcd.print("    ");
    lcd.setCursor(8, 2);
  } else if (rpm < 100) {
    lcd.print("   ");
    lcd.setCursor(7, 2);
  } else if (rpm < 1000) {
    lcd.print("  ");
    lcd.setCursor(6, 2);
  } else if (rpm < 10000) {
    lcd.print(" ");
    lcd.setCursor(5, 2);
  }
  lcd.print(rpm);

  // Printing temperature based on length
  lcd.setCursor(12, 3);
  if (temperature < 10) {
    lcd.print("  ");
    lcd.setCursor(14, 3);
  } else if (temperature < 100) {
    lcd.print(" ");
    lcd.setCursor(13, 3);
  } else {
    lcd.setCursor(12, 3);
  }
  lcd.print(temperature);
}

void loop() {
  // Read data from sensors & print data
  int current = updateCurrent();
  int voltage = getVoltage();
  int temp = (int) getCelcius();
  updateRPM();
  uint8_t buf[] = { current, voltage, temp };

  // Alarm if something got wrong
  if (current > 20 || voltage > 13 || temp > 45) {
    alarm();
  } else {
    lcd.backlight();
  }

  // Update values one time in 500ms
  if (millis() - prev_time_update > 500) {
    updateValues(current, voltage, rpm, temp);
    prev_time_update = millis();
    // Send values to the third pilot
    //Serial.write(buf, 3);
    
    // Write value based on the PSU temperature to the fans
    int value = temp * 255 / MAX_TEMP;
    if (value > 255) {
      value = 255;
    }
    if (value < 0) {
      value = 0;
    }
    analogWrite(pinPWM_OUT, value);
  }
}

float getVoltage() {
  // Get Voltage via voltage divider
  return (analogRead(pinV) * k / 1024.0) / (R2 / (R1 + R2));
}

float getCelcius() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  if ( !ds.search(addr)) {
    // No more addresses.
    ds.reset_search();
    return 0;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // CRC is not valid!
    return 0;
  }
  switch (addr[0]) {
    case 0x10:
      // Chip = DS18S20"
      type_s = 1;
      break;
    case 0x28:
      // Chip = DS18B20
      type_s = 0;
      break;
    case 0x22:
      // Chip = DS1822
      type_s = 0;
      break;
    default:
      // Device is not a DS18x20 family device.
      return 0;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44);
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  OneWire::crc8(data, 8);
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3;
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;
    else if (cfg == 0x20) raw = raw & ~3;
    else if (cfg == 0x40) raw = raw & ~1;
  }
  celsius = (float)raw / 16.0;
  return celsius;
}

void updateRPM() {
  if (millis() - lastmillis >= 1000) { /* Update every one second, this will be equal to reading frecuency (Hz).*/

    detachInterrupt(0);    //Disable interrupt when calculating
    total = 0;
    readings[index] = rpmcount * 60;  /* Convert frecuency to RPM, note: this works for one interruption per full rotation. For two interrups per full rotation use rpmcount * 30.*/

    for (int x = 0; x <= 9; x++) {
      total = total + readings[x];
    }

    average = total / numreadings;
    rpm = average;

    rpmcount = 0; // Restart the RPM counter
    index++;
    if (index >= numreadings) {
      index = 0;
    }

    lastmillis = millis(); // Uptade lasmillis
    attachInterrupt(0, rpm_fan, FALLING); //enable interrupt
  }
}

void rpm_fan() { /* this code will be executed every time the interrupt 0 (pin2) gets low.*/
  rpmcount++;
}


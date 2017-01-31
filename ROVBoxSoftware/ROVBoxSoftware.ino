#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

#define pinA          A0
#define pinV          A1
#define pinPWM_IN     A2
#define pinPWM_OUT     3

#define R1 100000 // Real Resistance on first resistor on voltage divider
#define R2 10000  // Real Resistance on second resistor on voltage divider

#define k 5.0 // Real Voltage between GND & 5V on arduino

LiquidCrystal_I2C lcd(0x27, 20, 4); // Initiate LCD display 16x2
OneWire ds(2);

void setup() {
  Serial.begin(115200);
  Wire.begin(); // I2C

  // LCD init
  lcd.init();            
  lcd.backlight();
  lcd.setCursor(10, 3);
  lcd.print("loading...")

  // Pins init
  pinMode(pinA, INPUT);
  pinMode(pinV, INPUT);
  pinMode(pinPWM_IN, INPUT);
  pinMode(pinPWM_OUT, OUTPUT);

  // Loading time
  delay(2500);
  lcd.clear();
}

void loop() {

  // Read data from sensors & print data
  int current = analogRead(pinA);
  int voltage = getVoltage();
  float temp = getCelcius();
  int rpm = analogRead(pinPWM_IN);
  updateLCD((int) current, (int) voltage, (int) temp, (int) rpm);

  // Write value based on the PSU temperature to the fans
  float value = temp / 60 * 255;
  if (value > 255) {
    value = 255;
  }
  if (value < 0) {
    value = 0;
  }
  analogWrite(pinPWM_OUT, value);
}

float getVoltage() {
  // Get Voltage via voltage divider
  return (analogRead(pinV) * k / 1024.0) / (R2/(R1+R2));
}

void updateLCD(int current, int voltage, int temp, int rpm) {
  Serial.println("Updating LCD display info");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("current: " + String(current) + "A");
  lcd.setCursor(0, 1);
  lcd.print("voltage: " + String(voltage) + "V");
  lcd.setCursor(0, 2);
  lcd.print("temperature: " + String(temp) + "C");
  lcd.setCursor(0, 3);
  lcd.print("rpm" + String(rpm));
}
float getCelcius() {
  Serial.println("Temperature sensor information:");
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return 0;
  }
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return 0;
  }
  Serial.println();
  switch (addr[0]) {
    case 0x10:
      Serial.println(" Chip = DS18S20"); 
      type_s = 1;
      break;
    case 0x28:
      Serial.println(" Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println(" Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return 0;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44); 
  delay(1000); 
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  Serial.print(" Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
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
  Serial.println("End of temperature sensor information");
}

#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("SCANNING I2C ON SPI PINS (6, 7)");
  
  // TFT_CS must be HIGH to deselect LCD!
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  
  Wire.begin(7, 6); // SDA=7 (MOSI), SCL=6 (SCLK)
  
  for(uint8_t address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("FOUND: 0x%02X\n", address);
    }
  }
  Serial.println("DONE");
}
void loop() {}

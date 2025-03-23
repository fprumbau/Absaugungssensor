#include "ADXL.h"
#include "Wire.h"

// Initialisierung des ADXL345
void ADXL::initADXL345() {
  // ADXL345 auf zweitem I²C-Bus initialisieren
  Wire1.begin(48, 47); // SDA = GPIO 48, SCL = GPIO 47
  Wire1.beginTransmission(ADXL345_ADDRESS);
  Wire1.write(ADXL345_REG_DEVID);
  Wire1.endTransmission();
  
  Wire1.requestFrom(ADXL345_ADDRESS, 1);
  if (Wire1.available()) {
    byte deviceID = Wire1.read();
    Serial.print("Device ID: 0x");
    Serial.println(deviceID, HEX);
    if (deviceID != 0xE5) Serial.println("ADXL345 not detected!");
  }
  
  writeRegister(ADXL345_ADDRESS, ADXL345_REG_POWER_CTL, 0x08, Wire1);
  writeRegister(ADXL345_ADDRESS, ADXL345_REG_DATA_FORMAT, 0x00, Wire1);
}

// Beschleunigungsdaten lesen
void ADXL::readAccelerometer() {
  uint8_t buffer[6];
  
  Wire1.beginTransmission(ADXL345_ADDRESS);
  Wire1.write(ADXL345_REG_DATAX0);
  Wire1.endTransmission();
  
  Wire1.requestFrom(ADXL345_ADDRESS, 6);
  for (int i = 0; i < 6; i++) {
    buffer[i] = Wire1.read();
  }
  
  accelX = (int16_t)((buffer[1] << 8) | buffer[0]);
  accelY = (int16_t)((buffer[3] << 8) | buffer[2]);
  accelZ = (int16_t)((buffer[5] << 8) | buffer[4]);
  
  gX = accelX * 0.0039;
  gY = accelY * 0.0039;
  gZ = accelZ * 0.0039;


}

void ADXL::print() {
    // Daten ausgeben
    Serial.print("Accel X: ");
    Serial.print(gX);
    Serial.print(" g, Y: ");
    Serial.print(gY);
    Serial.print(" g, Z: ");
    Serial.print(gZ);
    Serial.println(" g");
}

// Bewegungserkennung
bool ADXL::detectMovement(float threshold) {
  static float prevGX = 0, prevGY = 0, prevGZ = 0;
  bool movementDetected = false;
  
  float deltaX = abs(gX - prevGX);
  float deltaY = abs(gY - prevGY);
  float deltaZ = abs(gZ - prevGZ);
  
  if (deltaX > threshold || deltaY > threshold || deltaZ > threshold) {
    movementDetected = true;
  }
  
  prevGX = gX;
  prevGY = gY;
  prevGZ = gZ;
  
  return movementDetected;
}

// Register schreiben
void ADXL::writeRegister(uint8_t deviceAddress, uint8_t registerAddress, uint8_t value, TwoWire &wire1) {
  wire1.beginTransmission(deviceAddress);
  wire1.write(registerAddress);
  wire1.write(value);
  wire1.endTransmission();
}
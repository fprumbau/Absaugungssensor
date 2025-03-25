#include "ADXL.h"

ADXL::ADXL() : accelX(0), accelY(0), accelZ(0), gX(0.0), gY(0.0), gZ(0.0), initialized(false) {}

bool ADXL::init() {
  Wire1.begin(48, 47);
  Wire1.beginTransmission(ADXL345_ADDRESS);
  Wire1.write(ADXL345_REG_DEVID);
  Wire1.endTransmission();
  if (Wire1.requestFrom(ADXL345_ADDRESS, 1) == 1) {
    byte deviceID = Wire1.read();
    if (debugLevel & 8) {
      Serial.print("Device ID: 0x");
      Serial.println(deviceID, HEX);
    }
    if (deviceID == 0xE5) {
      writeRegister(ADXL345_REG_POWER_CTL, 0x08);
      writeRegister(ADXL345_REG_DATA_FORMAT, 0x00);
      // Initiale Messung zur Synchronisation
      readAccelerometer();
      initialized = true;
    } else {
      Serial.println("ADXL345 not detected!");
    }
  } else {
    Serial.println("I2C Error during init!");
  }
  return initialized;
}

void ADXL::writeRegister(uint8_t registerAddress, uint8_t value) {
  Wire1.beginTransmission(ADXL345_ADDRESS);
  Wire1.write(registerAddress);
  Wire1.write(value);
  Wire1.endTransmission();
}
void ADXL::readAccelerometer() {
  uint8_t buffer[6];
  Wire1.beginTransmission(ADXL345_ADDRESS);
  Wire1.write(ADXL345_REG_DATAX0);
  Wire1.endTransmission();
  if (Wire1.requestFrom(ADXL345_ADDRESS, 6) == 6) {
    for (int i = 0; i < 6; i++) {
      buffer[i] = Wire1.read();
    }
    accelX = (int16_t)((buffer[1] << 8) | buffer[0]);
    accelY = (int16_t)((buffer[3] << 8) | buffer[2]);
    accelZ = (int16_t)((buffer[5] << 8) | buffer[4]);
    prevGX = gX;
    gX = accelX * 0.0039;
    prevGY = gY;
    gY = accelY * 0.0039;
    prevGZ = gZ;
    gZ = accelZ * 0.0039;
  } else {
    Serial.println("I2C Error reading accelerometer!");
  }
}

bool ADXL::detectMovement(float threshold) {
  if (!initialized) {
    debugPrint(DEBUG_ADXL, "ADXL not initialized");
    return false;
  }
  float deltaX = abs(gX - prevGX);
  float deltaY = abs(gY - prevGY);
  float deltaZ = abs(gZ - prevGZ);

  bool movementDetected = false;
  if (deltaX > threshold) {
    debugPrint(DEBUG_ADXL, "DeltaX > threshold (" + String(deltaX, 2) + " > " + String(threshold, 2) + ")");
    movementDetected = true;
  }
  if (deltaY > threshold) {
    debugPrint(DEBUG_ADXL, "DeltaY > threshold (" + String(deltaY, 2) + " > " + String(threshold, 2) + ")");
    movementDetected = true;
  }
  if (deltaZ > threshold) {
    debugPrint(DEBUG_ADXL, "DeltaZ > threshold (" + String(deltaZ, 2) + " > " + String(threshold, 2) + ")");
    movementDetected = true;
  }
  return movementDetected;
}

void ADXL::sleep() {
  Wire1.beginTransmission(ADXL345_ADDRESS);
  Wire1.write(0x2D); // Power Control Register
  Wire1.write(0x00); // Sleep-Modus: Messung aus
  Wire1.endTransmission();
  debugPrint(DEBUG_ADXL, "ADXL entered sleep mode");
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

ADXL adxl; //Definition
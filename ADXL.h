#ifndef ADXL_H
#define ADXL_H

#include "global.h"

class ADXL {
public:
  ADXL();
  bool init();
  void print();
  void sleep(); // Neu: ADXL in Schlafmodus versetzen
  void readAccelerometer();
  bool detectMovement(float threshold);
  float getGX() const { return gX; }
  float getGY() const { return gY; }
  float getGZ() const { return gZ; }
  bool isInitialized() const { return initialized; }

private:
  static constexpr uint8_t ADXL345_ADDRESS = 0x53;
  static constexpr uint8_t ADXL345_REG_DEVID = 0x00;
  static constexpr uint8_t ADXL345_REG_POWER_CTL = 0x2D;
  static constexpr uint8_t ADXL345_REG_DATA_FORMAT = 0x31;
  static constexpr uint8_t ADXL345_REG_DATAX0 = 0x32;

  int16_t accelX, accelY, accelZ;
  float gX, gY, gZ;
  float prevGX, prevGY, prevGZ; // Statische Werte jetzt als Klassenvariablen
  bool initialized;
  bool firstRun; // Flag für ersten Durchlauf

  void writeRegister(uint8_t registerAddress, uint8_t value);
};

extern ADXL adxl;

#endif
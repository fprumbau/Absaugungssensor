#ifndef ADXL_H
#define ADXL_H

#include <Arduino.h>
#include "Wire.h"

// ADXL345 Definitionen
#define ADXL345_ADDRESS 0x53
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_POWER_CTL 0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0 0x32

class ADXL {

  private:
    int16_t accelX, accelY, accelZ;
    float gX, gY, gZ;

  public:
    void print();
    void initADXL345();
    void readAccelerometer();
    bool detectMovement(float threshold);
    void writeRegister(uint8_t deviceAddress, uint8_t registerAddress, uint8_t value, TwoWire &wire);
};

#endif
#ifndef VIBRATIONS_SENSOR_H
#define VIBRATIONS_SENSOR_H

#include "global.h"

struct VibrationProfilex {
  float freq;     // Hz
  float amp;     // in mg
  float var;      // Frequenzschwankung in %
  bool valid;
};

class VibrationSensor {
public:
  VibrationSensor();
  bool begin();
  void learn();                    // 10s Lernen → JSON
  bool movementDetected();         // FFT + Profil-Match
  void loadProfile();
  void saveProfile();
  void printProfile();
  void readAccelerometer(float &x, float &y, float &z);
  VibrationProfilex profile;

private:
  TwoWire wire1;
  bool readRaw(int16_t &x, int16_t &y, int16_t &z);
  float findFrequency(float samples[], int n);
  float rms(float samples[], int n);
  void collect(float samples[], int &count, int ms);
};

extern VibrationSensor vibrationSensor;

#endif
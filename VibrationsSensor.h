#ifndef VIBRATIONS_SENSOR_H
#define VIBRATIONS_SENSOR_H

#include "global.h"

struct VibrationProfile {
  float frequency;     // Hz
  float amplitude;     // in mg
  float variance;      // Frequenzschwankung in %
  bool valid;
};

class VibrationSensor {
public:
  VibrationSensor();
  bool begin();
  void learn();                    // 10s Lernmodus
  bool movementDetected();         // 1s Analyse
  void loadProfile();
  void saveProfile();
  void printProfile();
  VibrationProfile profile;

private:
  Adafruit_ADXL345_Unified accel;
  TwoWire* _wire;     // Pointer auf unseren custom Wire-Bus
  float analyzeFrequency(float samples[], int count, int sampleRate);
  float calculateRMS(float samples[], int count);
  void collectSamples(float samples[], int &count, int durationMs);
};

extern VibrationSensor vibrationSensor;

#endif
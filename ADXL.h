#include <sys/_stdint.h>
#ifndef ADXL_H
#define ADXL_H

#include "global.h"

struct VibrationProfile {
    char name[16] = "Unnamed";     // z.B. "Fräse 6mm"
    
    uint16_t freqHz      = 0;      // Grundfrequenz in Hz (z.B. 142 Hz)
    uint16_t amplitude   = 0;      // RMS-Amplitude ×100 (z.B. 2350 = 23.5)
    uint16_t freqVar     = 0;      // Frequenz-Varianz ×100 (z.B. 80 = ±0.8Hz)
    uint16_t ampVar      = 0;      // Amplituden-Varianz ×100
    
    bool     valid       = false;  // genug Messpunkte + stabile Werte
};

class ADXL {
public:
  ADXL();
  bool init();
  void sleep(); // Neu: ADXL in Schlafmodus versetzen
  bool isInitialized() const { return initialized; }
  float getX();
  bool learn(const char* toolName); // 10s Lernen → JSON
  bool isRunning();
  bool loadProfile();
  void saveProfile();
  void printProfile() const;
  void readAccelerometer(float &x, float &y, float &z);
  VibrationProfile profile;

private:
  float x;             
  bool initialized;
  void writeRegister(uint8_t registerAddress, uint8_t value);
  bool readRaw(int16_t &x, int16_t &y, int16_t &z);
  float findFrequency(float samples[], int n);
  float rms(float samples[], int n);
  void collect(float samples[], int &count, int ms);
  float detectDominantFrequency(const int16_t*, int, int);
  bool readSamples(int16_t* buffer, uint16_t count, uint32_t sampleDelayUs);
  bool i2cRead(uint8_t reg, uint8_t* data, uint8_t len);
};

extern ADXL adxl;

#endif
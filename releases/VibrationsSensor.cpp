#include "VibrationsSensor.h"
#include <Adafruit_BusIO_Register.h>  // Für den Hack

#define ADDR 0x53
#define SAMPLES 128
#define RATE 100

VibrationSensor::VibrationSensor() : wire1(TwoWire(1)), profile{0,0,0,false} {
  //EEPROM.begin(512);
}

bool VibrationSensor::begin() {
  /*wire1.begin(48, 47);
  wire1.setClock(400000);

  // Device ID prüfen
  wire1.beginTransmission(ADDR);
  wire1.write(0x00);
  if (wire1.endTransmission() != 0) return false;
  wire1.requestFrom(ADDR, 1);
  if (wire1.read() != 0xE5) return false;

  // ADXL aktivieren
  wire1.beginTransmission(ADDR);
  wire1.write(0x2D); 
  wire1.write(0x08);
  wire1.endTransmission();
  wire1.beginTransmission(ADDR);
  wire1.write(0x31); 
  wire1.write(0x0B);
  wire1.endTransmission();
  */

  wire1.begin(48, 47);
  wire1.beginTransmission(ADDR);
  wire1.write(ADXL345_REG_DEVID);
  wire1.endTransmission();
  if (Wire1.requestFrom(ADDR, 1) == 1) {
    byte deviceID = Wire1.read();
    if (debugLevel & 8) {
      Serial.print("Device ID: 0x");
      Serial.println(deviceID, HEX);
    }
    if (deviceID == 0xE5) {
        wire1.beginTransmission(ADDR);
        wire1.write(ADXL345_REG_POWER_CTL);
        wire1.write(0x08);
        wire1.endTransmission();
        wire1.beginTransmission(ADDR);
        wire1.write(ADXL345_REG_DATA_FORMAT);
        wire1.write(0x00);
        wire1.endTransmission();
    } 
  } else {
    Serial.println("I2C Error during init!");
  }

  Serial.println("ADXL345 bereit (Wire1)");
  loadProfile();
  return true;
}

bool VibrationSensor::readRaw(int16_t &x, int16_t &y, int16_t &z) {
  wire1.beginTransmission(ADDR);
  wire1.write(0x32);
  wire1.endTransmission();
  if (wire1.requestFrom(ADDR, 6) != 6) return false;
  x = wire1.read() | (wire1.read() << 8);
  y = wire1.read() | (wire1.read() << 8);
  z = wire1.read() | (wire1.read() << 8);
  return true;
}

void VibrationSensor::readAccelerometer(float &x, float &y, float &z) {
  uint8_t buffer[6];
  Wire1.beginTransmission(ADDR);
  Wire1.write(ADXL345_REG_DATAX0);
  Wire1.endTransmission();
  if (Wire1.requestFrom(ADDR, 6) == 6) {
    for (int i = 0; i < 6; i++) {
      buffer[i] = Wire1.read();
    }
    int16_t accelX = (int16_t)((buffer[1] << 8) | buffer[0]);
    int16_t accelY = (int16_t)((buffer[3] << 8) | buffer[2]);
    int16_t accelZ = (int16_t)((buffer[5] << 8) | buffer[4]);
    x = accelX * 0.0039;
    y = accelY * 0.0039;
    z = accelZ * 0.0039;
  } else {
    Serial.println("I2C Error reading accelerometer!");
  }
}

void VibrationSensor::collect(float samples[], int &count, int ms) {
  count = 0;
  unsigned long end = millis() + ms;
  int16_t x, y, z;
  while (millis() < end && count < SAMPLES) {
    if (readRaw(x, y, z)) {
      samples[count++] = (z * 0.0039) - 9.81;  // nur Z, Grav abziehen
    }
    delay(1000 / RATE);
  }
}

float VibrationSensor::rms(float samples[], int n) {
  if (n == 0) return 0;
  float sum = 0;
  for (int i = 0; i < n; i++) sum += samples[i] * samples[i];
  return sqrt(sum / n) * 1000;
}

float VibrationSensor::findFrequency(float samples[], int n) {
  if (n < 20) return 0;
  float maxCorr = 0;
  int bestLag = 0;
  for (int lag = 8; lag < 40; lag++) {  // 25–125 Hz
    float corr = 0;
    for (int i = 0; i < n - lag; i++) {
      corr += samples[i] * samples[i + lag];
    }
    if (corr > maxCorr) { maxCorr = corr; bestLag = lag; }
  }
  return bestLag ? (float)RATE / bestLag : 0;
}

void VibrationSensor::learn() {
  Serial.println("LERNE 10s – Werkzeug starten!");
  float freqs[10] = {0}, amps[10] = {0};
  int valid = 0;

  for (int i = 0; i < 10; i++) {
    float buf[SAMPLES];
    int cnt;
    collect(buf, cnt, 1000);

    float f = findFrequency(buf, cnt);
    float a = rms(buf, cnt);

    if (a > 80 && f > 20) {
      freqs[valid] = f;
      amps[valid++] = a;
      Serial.printf("Segment %d: %.1f Hz, %.0f mg\n", i+1, f, a);
    } else {
      Serial.println("Stille → Lernen beendet.");
      break;
    }
    delay(200);
  }

  if (valid < 2) { Serial.println("Kein Profil!"); profile.valid = false; return; }

  float sumF = 0, sumA = 0, var = 0;
  for (int i = 0; i < valid; i++) { sumF += freqs[i]; sumA += amps[i]; }
  profile.freq = sumF / valid;
  profile.amp = sumA / valid;
  for (int i = 0; i < valid; i++) var += sq(freqs[i] - profile.freq);
  profile.var = valid > 1 ? sqrt(var/(valid-1)) / profile.freq * 100 : 10;
  profile.valid = true;

  saveProfile();
  printProfile();
}

bool VibrationSensor::movementDetected() {
  if (!profile.valid) return false;

  float buf[SAMPLES];
  int cnt;
  collect(buf, cnt, 1000);

  float f = findFrequency(buf, cnt);
  float a = rms(buf, cnt);

  bool match = fabs(f - profile.freq) < profile.freq * profile.var / 100 &&
               a > profile.amp * 0.7;

  if (match) Serial.printf("ERKANNT: %.1f Hz (Soll: %.1f)\n", f, profile.freq);
  return match;
}

void VibrationSensor::saveProfile() {
  EEPROM.put(0, profile);
  EEPROM.commit();
}

void VibrationSensor::loadProfile() {
  EEPROM.get(0, profile);
  if (profile.freq < 10 || profile.freq > 200) profile.valid = false;
}

void VibrationSensor::printProfile() {
  DynamicJsonDocument doc(200);
  doc["tool"] = "Schleifer";
  doc["freq"] = round(profile.freq * 10)/10;
  doc["amp"] = round(profile.amp);
  doc["var"] = round(profile.var * 10)/10;
  serializeJsonPretty(doc, Serial);
  Serial.println();
}

VibrationSensor vibrationSensor;
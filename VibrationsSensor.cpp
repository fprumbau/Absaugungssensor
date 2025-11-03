#include "VibrationsSensor.h"

#define SAMPLE_RATE 100    // 100 Hz
#define FFT_N 128          // FFT Länge
#define PROFILE_ADDR 0

VibrationSensor::VibrationSensor() : accel(12345), profile{0, 0, 0, false} {}

bool VibrationSensor::begin() {
  if (!accel.begin()) {
    Serial.println("ADXL345 nicht gefunden!");
    return false;
  }
  accel.setRange(ADXL345_RANGE_16_G);
  accel.setDataRate(ADXL345_DATARATE_100_HZ);
  loadProfile();
  return true;
}

void VibrationSensor::collectSamples(float samples[], int &count, int durationMs) {
  count = 0;
  unsigned long start = millis();
  float lastZ = 0;
  while (millis() - start < durationMs && count < FFT_N * 2) {
    sensors_event_t event;
    accel.getEvent(&event);
    float z = event.acceleration.z - 9.81; // Gravitation entfernen
    float delta = abs(z - lastZ);
    if (delta > 0.3) {  // Nur signifikante Änderungen
      samples[count++] = z;
    }
    lastZ = z;
    delay(1000 / SAMPLE_RATE);
  }
}

float VibrationSensor::calculateRMS(float samples[], int count) {
  if (count == 0) return 0;
  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += samples[i] * samples[i];
  }
  return sqrt(sum / count) * 1000; // in mg
}

float VibrationSensor::analyzeFrequency(float samples[], int count, int sampleRate) {
  if (count < 10) return 0;

  // Einfache Autokorrelation für Grundfrequenz
  int maxLag = sampleRate / 5; // max 20 Hz
  float maxCorr = 0;
  int bestLag = 0;

  for (int lag = 5; lag < maxLag; lag++) {
    float corr = 0;
    int pairs = 0;
    for (int i = 0; i < count - lag; i++) {
      corr += samples[i] * samples[i + lag];
      pairs++;
    }
    if (pairs > 0) corr /= pairs;
    if (corr > maxCorr) {
      maxCorr = corr;
      bestLag = lag;
    }
  }

  if (bestLag > 0) {
    return (float)sampleRate / bestLag;
  }
  return 0;
}

void VibrationSensor::learn() {
  Serial.println("LERNMODUS: Werkzeug 10s lang betreiben...");
  float allSamples[FFT_N * 10];
  int totalCount = 0;
  float freqs[10] = {0};
  float amps[10] = {0};
  int validSegments = 0;

  for (int seg = 0; seg < 10; seg++) {
    float segment[FFT_N];
    int count = 0;
    collectSamples(segment, count, 1000);

    float amp = calculateRMS(segment, count);
    float freq = analyzeFrequency(segment, count, SAMPLE_RATE);

    // Nur signifikante Schwingung speichern
    if (amp > 50 && freq > 5 && freq < 100) {
      freqs[validSegments] = freq;
      amps[validSegments] = amp;
      validSegments++;
      Serial.printf("Segment %d: %.1f Hz, %.0f mg\n", seg + 1, freq, amp);
    } else {
      Serial.printf("Segment %d: Stille\n", seg + 1);
      if (validSegments > 2) {
        Serial.println("Genug Daten → Lernen beendet.");
        break;
      }
    }
    delay(100);
  }

  if (validSegments < 2) {
    Serial.println("Kein klares Profil erkannt!");
    profile.valid = false;
    return;
  }

  // Mittelwert + Varianz
  float sumFreq = 0, sumAmp = 0;
  for (int i = 0; i < validSegments; i++) {
    sumFreq += freqs[i];
    sumAmp += amps[i];
  }
  float avgFreq = sumFreq / validSegments;
  float avgAmp = sumAmp / validSegments;

  float varSum = 0;
  for (int i = 0; i < validSegments; i++) {
    varSum += pow(freqs[i] - avgFreq, 2);
  }
  float variance = validSegments > 1 ? sqrt(varSum / (validSegments - 1)) / avgFreq * 100 : 10;

  profile = { avgFreq, avgAmp, variance, true };
  saveProfile();
  printProfile();

  Serial.println("PROFIL GESPEICHERT!");
}

bool VibrationSensor::movementDetected() {
  if (!profile.valid) return false;

  float samples[FFT_N];
  int count = 0;
  collectSamples(samples, count, 1000);

  float amp = calculateRMS(samples, count);
  float freq = analyzeFrequency(samples, count, SAMPLE_RATE);

  bool freqMatch = freq > profile.frequency * (1 - profile.variance / 100) &&
                   freq < profile.frequency * (1 + profile.variance / 100);
  bool ampMatch = amp > profile.amplitude * 0.6;

  bool detected = freqMatch && ampMatch && amp > 50;

  if (detected) {
    Serial.printf("BEWEGUNG ERKANNT: %.1f Hz, %.0f mg\n", freq, amp);
  }

  return detected;
}

void VibrationSensor::saveProfile() {
  EEPROM.begin(512);
  EEPROM.put(PROFILE_ADDR, profile);
  EEPROM.commit();
  EEPROM.end();
}

void VibrationSensor::loadProfile() {
  EEPROM.begin(512);
  EEPROM.get(PROFILE_ADDR, profile);
  EEPROM.end();
  if (profile.frequency < 1 || profile.frequency > 200) {
    profile.valid = false;
  }
}

void VibrationSensor::printProfile() {
  DynamicJsonDocument doc(256);
  doc["tool"] = "unknown";
  doc["frequency"] = round(profile.frequency * 10) / 10.0;
  doc["amplitude_mg"] = round(profile.amplitude);
  doc["variance_pct"] = round(profile.variance * 10) / 10.0;
  doc["timestamp"] = millis();

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

VibrationSensor vibrationSensor;
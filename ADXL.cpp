#include "ADXL.h"

#define ADDR 0x53
#define SAMPLES 128
#define RATE 50 // 20ms pro Sample → 50 Hz → STABIL!

ADXL::ADXL() : initialized(false) {}

bool ADXL::init() {
  Wire1.begin(48, 47);
  Wire1.beginTransmission(ADDR);
  Wire1.write(ADXL345_REG_DEVID);
  Wire1.endTransmission();
  if (Wire1.requestFrom(ADDR, 1) == 1) {
    byte deviceID = Wire1.read();
    if (debugLevel & 8) {
      Serial.print("Device ID: 0x");
      Serial.println(deviceID, HEX);
    }
    if (deviceID == 0xE5) {
      writeRegister(ADXL345_REG_POWER_CTL, 0x08);
      writeRegister(ADXL345_REG_DATA_FORMAT, 0x00);
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
  Wire1.beginTransmission(ADDR);
  Wire1.write(registerAddress);
  Wire1.write(value);
  Wire1.endTransmission();
}

void ADXL::sleep() {
  if (!initialized) {
    debugPrint(DEBUG_ADXL, "ADXL not initialized");
    return;
  }  
  Wire1.beginTransmission(ADDR);
  Wire1.write(0x2D); // Power Control Register
  Wire1.write(0x00); // Sleep-Modus: Messung aus
  Wire1.endTransmission();
  debugPrint(DEBUG_ADXL, "ADXL entered sleep mode");
}

void ADXL::readAccelerometer(float &x, float &y, float &z) {
  if (!initialized) {
    debugPrint(DEBUG_ADXL, "ADXL not initialized");
    return;
  }
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
    this->x = x = accelX * 0.0039;
    y = accelY * 0.0039;
    z = accelZ * 0.0039;
  } else {
    Serial.println("I2C Error reading accelerometer!");
  }
}

bool ADXL::readRaw(int16_t &x, int16_t &y, int16_t &z) {
  Wire1.beginTransmission(ADDR);
  Wire1.write(0x32);
  Wire1.endTransmission();
  if (Wire1.requestFrom(ADDR, 6) != 6) return false;
  x = Wire1.read() | (Wire1.read() << 8);
  y = Wire1.read() | (Wire1.read() << 8);
  z = Wire1.read() | (Wire1.read() << 8);
  return true;
}

void ADXL::collect(float samples[], int &count, int ms) {
  count = 0;
  unsigned long end = millis() + ms;
  int16_t x, y, z;
  while (millis() < end && count < SAMPLES) {
    if (readRaw(x, y, z)) {
      samples[count++] = (z * 0.0039) - 1.0; //9.81;  // nur Z, Grav abziehen
    }
    // KEIN delay(1000/RATE) mehr!
    // I2C ist langsam genug!
    //delay(1000 / RATE);
  }
}

float ADXL::rms(float samples[], int n) {
  if (n == 0) return 0;
  float sum = 0;
  for (int i = 0; i < n; i++) sum += samples[i] * samples[i];
  return sqrt(sum / n) * 1000;
}

float ADXL::findFrequency(float samples[], int n) {
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

bool ADXL::learn(const char* toolName)
{
    Serial.printf("Sammle %d Samples @ %d Hz...\n", count, RATE);

    const uint16_t SAMPLE_TIME_MS = 1000;
    const uint16_t SAMPLE_RATE_HZ = 500;
    const uint16_t N = SAMPLE_RATE_HZ;

    int16_t raw[N];
    if (!readSamples(raw, N, 1000000 / SAMPLE_RATE_HZ)) {
        Serial.println(F("Fehler beim Aufnehmen"));
        return false;
    }

    // 1. FFT für Grundfrequenz
    float freq = detectDominantFrequency(raw, N, SAMPLE_RATE_HZ);
    
    // 2. Amplitude (RMS)
    float rms = 0;
    for (int i = 0; i < N; i++) rms += raw[i] * raw[i];
    rms = sqrt(rms / N);
    uint16_t amplitude = (uint16_t)(rms * 100);

    // 3. Varianz über 4 × 250ms-Blöcke
    uint16_t freqs[4] = {0};
    uint16_t amps[4]  = {0};
    for (int b = 0; b < 4; b++) {
        int start = b * (N/4);
        freqs[b] = (uint16_t)detectDominantFrequency(&raw[start], N/4, SAMPLE_RATE_HZ);
        float blockRms = 0;
        for (int i = 0; i < N/4; i++) blockRms += raw[start+i] * raw[start+i];
        blockRms = sqrt(blockRms / (N/4));
        amps[b] = (uint16_t)(blockRms * 100);
    }

    // Varianz berechnen
    uint32_t fsum = 0, asum = 0;
    for (int i = 0; i < 4; i++) { fsum += freqs[i]; asum += amps[i]; }
    uint16_t favg = fsum / 4;
    uint16_t aavg = asum / 4;

    uint32_t fvar = 0, avar = 0;
    for (int i = 0; i < 4; i++) {
        int df = freqs[i] - favg;
        int da = amps[i] - aavg;
        fvar += df * df;
        avar += da * da;
    }
    uint16_t freqVar = (uint16_t)sqrt(fvar / 4.0) * 100;
    uint16_t ampVar  = (uint16_t)sqrt(avar / 4.0) * 100;

    // 4. Validierung
    bool valid = (freq > 20 && freq < 500 && amplitude > 300 && freqVar < 200);

    // 5. Speichern
    strlcpy(profile.name, toolName, sizeof(profile.name));
    profile.freqHz    = (uint16_t)freq;
    profile.amplitude = amplitude;
    profile.freqVar   = freqVar;
    profile.ampVar    = ampVar;
    profile.valid     = valid;

    Serial.printf("[LEARN] %s → %d Hz ±%d, Amp %d.%02d ±%d\n",
        profile.name,
        profile.freqHz,
        profile.freqVar,
        profile.amplitude / 100, profile.amplitude % 100,
        profile.ampVar
    );

    return valid;
}

void ADXL::saveProfile()
{
    char json[256];
    snprintf(json, sizeof(json),
        "{\"name\":\"%s\",\"freq\":%d,\"amp\":%d,\"fvar\":%d,\"avar\":%d,\"valid\":%d}",
        profile.name,
        profile.freqHz,
        profile.amplitude,
        profile.freqVar,
        profile.ampVar,
        profile.valid
    );
    config.setValue("VIB_Profile", json, true);
    Serial.println(F("[SAVE] Profil gespeichert"));
}

bool ADXL::loadProfile()
{
    const char* json = config.getProfile();
    if (!json || strlen(json) < 20) return false;

    // Einfaches Parsen mit sscanf
    char name[16];
    int f, a, fv, av, v;
    if (sscanf(json, "{\"name\":\"%15[^\"]\",\"freq\":%d,\"amp\":%d,\"fvar\":%d,\"avar\":%d,\"valid\":%d}",
               name, &f, &a, &fv, &av, &v) == 6) {
        strlcpy(profile.name, name, sizeof(profile.name));
        profile.freqHz = f;
        profile.amplitude = a;
        profile.freqVar = fv;
        profile.ampVar = av;
        profile.valid = v;
        Serial.printf("[LOAD] %s geladen\n", profile.name);
        return true;
    }
    return false;
}

void ADXL::printProfile() const
{
    DynamicJsonDocument doc(512);
    JsonObject root = doc.to<JsonObject>();

    // ── Metadaten ─────────────────────────────────────
    root["tool"]        = profile.name;
    root["valid"]       = profile.valid;
    root["learnedAt"]   = millis();                    // optional: Zeitstempel

    // ── Physikalische Kennwerte ───────────────────────
    JsonObject freq = root.createNestedObject("frequency");
    freq["hz"]          = profile.freqHz;
    freq["tolerance"]   = profile.freqVar / 100.0f;    // als float

    JsonObject amp = root.createNestedObject("amplitude");
    amp["value"]        = profile.amplitude / 100.0f;  // 23.45
    amp["tolerance"]    = profile.ampVar / 100.0f;

    // ── Rohdaten (optional ein-/ausblendbar) ──────────
    // JsonArray samples = root.createNestedArray("rawSamples");
    // for (uint16_t i = 0; i < profile.sampleCount; i++)
    //     samples.add(profile.samples[i]);

    // ── Ausgabe ───────────────────────────────────────
    serializeJsonPretty(root, Serial);
    Serial.println();  // Leerzeile für sauberen Cut&Paste
}

bool ADXL::isRunning()
{
    if (!profile.valid) return false;

    int16_t test[64];
    if (!readSamples(test, 64, 2000)) return false;

    float freq = detectDominantFrequency(test, 64, 500);
    float rms = 0;
    for (int i = 0; i < 64; i++) rms += test[i] * test[i];
    rms = sqrt(rms / 64) * 100;

    bool freqOk = abs((int)freq - profile.freqHz) <= (profile.freqVar + 30);
    bool ampOk  = abs((int)rms - profile.amplitude) <= (profile.ampVar + 200);

    return freqOk && ampOk;
}

float ADXL::detectDominantFrequency(const int16_t* samples, int n, int sampleRate)
{
    float maxMag = 0;
    float bestFreq = 0;
    for (int k = 5; k < 100; k++) {  // 50..1000 Hz
        float freq = k * sampleRate / (float)n;
        float cosVal = cos(2 * PI * k / n);
        float sinVal = sin(2 * PI * k / n);
        float Q1 = 0, Q2 = 0;
        for (int i = 0; i < n; i++) {
            float Q0 = samples[i] + 2 * cosVal * Q1 - Q2;
            Q2 = Q1; Q1 = Q0;
        }
        float mag = sqrt(Q1*Q1 + Q2*Q2 - 2*Q1*Q2*cosVal);
        if (mag > maxMag) { maxMag = mag; bestFreq = freq; }
    }
    return bestFreq;
}

// ADXL.cpp – füge diese Methode in deine Klasse ein
bool ADXL::readSamples(int16_t* buffer, uint16_t count, uint32_t sampleDelayUs)
{
    if (!buffer || count == 0 || count > 1024) return false;

    // ADXL345 Register
    const uint8_t REG_DATA = 0x32;   // DATAX0 (6 Byte ab hier)

    for (uint16_t i = 0; i < count; ++i) {
        uint8_t raw[6];
        if (!i2cRead(REG_DATA, raw, 6)) {
            Serial.println(F("[ADXL] I2C Fehler beim Lesen"));
            return false;
        }

        // ADXL345 liefert 10-Bit signed, little-endian
        int16_t x = (int16_t)(raw[1] << 8) | raw[0];
        int16_t y = (int16_t)(raw[3] << 8) | raw[2];
        int16_t z = (int16_t)(raw[5] << 8) | raw[4];

        // Wir nutzen die **stärkste Achse** → wie ein echter Vibrationssensor
        int16_t mag = abs(x);
        if (abs(y) > mag) mag = abs(y);
        if (abs(z) > mag) mag = abs(z);

        buffer[i] = mag;

        if (sampleDelayUs > 0) {
            uint32_t start = micros();
            while (micros() - start < sampleDelayUs) {
                yield();  // ESP32 bleibt responsiv
            }
        }
    }

    return true;
}

// In ADXL.cpp – einmalig
bool ADXL::i2cRead(uint8_t reg, uint8_t* data, uint8_t len)
{
    Wire.beginTransmission(ADDR);  // 0x53
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;  // repeated start

    Wire.requestFrom(ADDR, len);
    if (Wire.available() < len) return false;

    for (uint8_t i = 0; i < len; ++i)
        data[i] = Wire.read();

    return true;
}

float ADXL::getX() {
    return x;
}

ADXL adxl; //Definition
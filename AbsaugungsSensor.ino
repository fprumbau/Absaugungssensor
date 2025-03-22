#include <Wire.h>
#include "SSD1306Wire.h"
#include <Adafruit_NeoPixel.h>

// Display-Definitionen (Standardpins beim Heltec LoRa WiFi 32 V3)
#define SDA_OLED 4  // Standard-SDA-Pin für Heltec OLED
#define SCL_OLED 15 // Standard-SCL-Pin für Heltec OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C // Typische Adresse für Heltec OLED
SSD1306Wire display(OLED_ADDRESS, SDA_OLED, SCL_OLED);

// ADXL345 Definitionen
#define ADXL345_ADDRESS 0x53
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_POWER_CTL 0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0 0x32

// NeoPixel Definitionen
#define PIN_NEO_PIXEL  12
#define NUM_PIXELS     1
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

// Pin-Definitionen
const int RelaisPin = 2;
const int TasterPin = 13;

// Variablen
int16_t accelX, accelY, accelZ;
float gX, gY, gZ;
unsigned long ZeitAus;
bool RelaisState = false;
bool TasterGedrueckt = false;
unsigned long TasterPressTime = 0;
unsigned long RelaisEinZeit = 0;
const long TasterEntprellZeit = 50;
const long RelaisNachlaufzeit = 10000; // 10 Sekunden Nachlaufzeit

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Display auf Standard-I²C-Bus initialisieren
  Wire.begin(SDA_OLED, SCL_OLED);
  display.init();
  display.flipScreenVertically(); // Oft nötig bei Heltec-Displays
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.drawString(0, 0, "AbsaugungsSensor startet...");
  display.display();

  // ADXL345 auf zweitem I²C-Bus initialisieren (Wire1 ist vordefiniert)
  Wire1.begin(48, 47); // SDA = GPIO 48, SCL = GPIO 47
  initADXL345();

  // NeoPixel initialisieren
  pixels.begin();
  pixels.setBrightness(50);
  pixels.clear();
  pixels.show();

  // Pin-Modi setzen
  pinMode(RelaisPin, OUTPUT);
  digitalWrite(RelaisPin, LOW);
  pinMode(TasterPin, INPUT_PULLUP);
  ZeitAus = millis();
}

// Initialisierung des ADXL345
void initADXL345() {
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

// Register schreiben
void writeRegister(uint8_t deviceAddress, uint8_t registerAddress, uint8_t value, TwoWire &wire) {
  wire.beginTransmission(deviceAddress);
  wire.write(registerAddress);
  wire.write(value);
  wire.endTransmission();
}

// Beschleunigungsdaten lesen
void readAccelerometer() {
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

// Bewegungserkennung
bool detectMovement(float threshold) {
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

void loop() {
  // Taster-Logik
  bool TasterState = digitalRead(TasterPin) == LOW;

  if (TasterState && !TasterGedrueckt) {
    unsigned long currentTime = millis();
    if (currentTime - TasterPressTime > TasterEntprellZeit) {
      TasterGedrueckt = true;
      TasterPressTime = currentTime;
      RelaisState = !RelaisState;
      digitalWrite(RelaisPin, RelaisState ? HIGH : LOW);
      if (RelaisState) {
        RelaisEinZeit = currentTime;
      }
      pixels.setPixelColor(0, pixels.Color(0, 150, 0));
      pixels.show();
    }
  } else if (!TasterState && TasterGedrueckt) {
    TasterGedrueckt = false;
    pixels.setPixelColor(0, pixels.Color(150, 0, 0));
    pixels.show();
  }

  // Nachlaufzeit-Logik
  if (RelaisState && (millis() - RelaisEinZeit > RelaisNachlaufzeit)) {
    RelaisState = false;
    digitalWrite(RelaisPin, LOW);
    pixels.setPixelColor(0, pixels.Color(150, 0, 0));
    pixels.show();
  }

  // Beschleunigungsdaten lesen
  readAccelerometer();
  
  // Daten ausgeben
  Serial.print("Accel X: ");
  Serial.print(gX);
  Serial.print(" g, Y: ");
  Serial.print(gY);
  Serial.print(" g, Z: ");
  Serial.print(gZ);
  Serial.println(" g");
  
  // Bewegung testen
  if (detectMovement(0.2)) {
    Serial.println("Bewegung erkannt!");
    // Optional: Relais bei Bewegung einschalten
    if (!RelaisState) {
      RelaisState = true;
      digitalWrite(RelaisPin, HIGH);
      RelaisEinZeit = millis();
      pixels.setPixelColor(0, pixels.Color(0, 150, 0));
      pixels.show();
    }
  }

  // Display aktualisieren
  display.clear();
  display.drawString(0, 0, "AbsaugungsSensor");
  display.drawString(0, 16, "Relais: " + String(RelaisState ? "Ein" : "Aus"));
  display.drawString(0, 32, "X: " + String(gX) + " g");
  display.drawString(0, 42, "Y: " + String(gY) + " g");
  display.drawString(0, 52, "Z: " + String(gZ) + " g");
  display.display();

  delay(500);
}
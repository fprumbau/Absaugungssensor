#include <Wire.h>
#include "SSD1306Wire.h"
#include "pins_arduino.h"

#define TASTER 19
SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED); // ADDRESS, SDA, SCL

void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void displayReset(void) {
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, HIGH);
  delay(1);
  digitalWrite(RST_OLED, LOW);
  delay(1);
  digitalWrite(RST_OLED, HIGH);
  delay(1);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("Start vor Display-Initialisierung...");

  // Heltec-spezifische Display-Steuerung
  VextON();
  displayReset();

  // Display initialisieren
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  Serial.println("Display-Initialisierung abgeschlossen");

  // Taster konfigurieren
  pinMode(TASTER, INPUT_PULLUP);
}

void loop() {
  static unsigned long seconds = 0;
  static unsigned long lastSecond = 0;
  static int lastStableState = HIGH;
  static int lastReading = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 100;

  // Sekundenzähler aktualisieren
  if (millis() - lastSecond >= 1000) {
    seconds++;
    lastSecond = millis();
  }

  // Zustand des Tasters lesen (mit Entprellung)
  int reading = digitalRead(TASTER);
  if (reading != lastReading) {
    lastDebounceTime = millis();
    lastReading = reading;
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int currentState = digitalRead(TASTER);
    if (currentState != lastStableState) {
      Serial.print("GPIO 19 Zustand stabil geändert: ");
      Serial.println(currentState);
      lastStableState = currentState;
      lastReading = currentState;
    }
  }

  // Display aktualisieren
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Board laeuft...");
  display.drawString(0, 15, "Sekunden: " + String(seconds));
  display.drawString(0, 30, "Taster: " + String(digitalRead(TASTER) == HIGH ? "OFF" : "ON"));
  display.display();

  delay(100);
}
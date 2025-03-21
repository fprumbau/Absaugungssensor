#include <Wire.h>
#include "SSD1306Wire.h"
#include "pins_arduino.h"

#define TASTER 19
#define DEBOUNCE_DELAY 100

SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED);

void VextON() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void displayReset() {
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
  Serial.println("Starting Absaugungssensor...");

  VextON();
  displayReset();

  if (!display.init()) {
    Serial.println("OLED initialization failed!");
    while (1) delay(100);
  }
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  Serial.println("OLED initialized");

  pinMode(TASTER, INPUT_PULLUP);
}

void loop() {
  static unsigned long seconds = 0;
  static unsigned long lastSecond = 0;
  static int lastStableState = HIGH;
  static int lastReading = HIGH;
  static unsigned long lastDebounceTime = 0;

  // Sekundenzähler aktualisieren
  if (millis() - lastSecond >= 1000) {
    seconds++;
    lastSecond = millis();
  }

  // Tasterstatus mit Entprellung lesen
  int reading = digitalRead(TASTER);
  if (reading != lastReading) {
    lastDebounceTime = millis();
    lastReading = reading;
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    int currentState = digitalRead(TASTER);
    if (currentState != lastStableState) {
      Serial.print("GPIO 19 state changed: ");
      Serial.println(currentState);
      lastStableState = currentState;
      lastReading = currentState;
    }
  }

  // Display aktualisieren
  char buffer[20];
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Board laeuft...");
  sprintf(buffer, "Sekunden: %lu", seconds);
  display.drawString(0, 15, buffer);
  display.drawString(0, 30, digitalRead(TASTER) == HIGH ? "Taster: OFF" : "Taster: ON");
  display.display();

  delay(100);
}
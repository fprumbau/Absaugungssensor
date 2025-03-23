#include "display.h"

Display::Display() : oled(OLED_ADDRESS, SDA_OLED, SCL_OLED) {}

bool Display::init() {
  pinMode(VEXT_PIN, OUTPUT);
  vextOn(); // Vext einschalten für Display
  Wire.begin(SDA_OLED, SCL_OLED);
  oled.init();
  oled.flipScreenVertically();
  oled.setFont(ArialMT_Plain_10);
  debugPrint(DEBUG_DISPLAY, "Display initialized");
  return true; // Hier könnte eine Fehlerprüfung hinzugefügt werden
}

void Display::reset() {
  vextOff();
  delay(1); // Kurze Pause für Reset
  vextOn();
  oled.init();
  oled.flipScreenVertically();
  oled.setFont(ArialMT_Plain_10);
  debugPrint(DEBUG_DISPLAY, "Display reset");
}

void Display::vextOn() {
  digitalWrite(VEXT_PIN, LOW); // Heltec: LOW schaltet Vext ein
}

void Display::vextOff() {
  digitalWrite(VEXT_PIN, HIGH); // Heltec: HIGH schaltet Vext aus
}

void Display::clear() {
  oled.clear();
}

void Display::drawString(int x, int y, const String& text) {
  oled.drawString(x, y, text);
}

void Display::display() {
  oled.display();
}

Display oled; // Definition des globalen Objekts
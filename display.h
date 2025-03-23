#ifndef DISPLAY_H
#define DISPLAY_H

#define OLED_ADDRESS 0x3C

#include "global.h"
#include "SSD1306Wire.h"

class Display {
public:
  Display();
  bool init();              // Initialisiert das Display
  void reset();             // Führt einen Display-Reset durch
  void vextOn();            // Schaltet Vext (Stromversorgung) ein
  void vextOff();           // Schaltet Vext aus
  void clear();             // Löscht das Display
  void drawString(int x, int y, const String& text); // Zeichnet Text
  void display();           // Aktualisiert das Display

private:
  SSD1306Wire oled;         // Internes SSD1306Wire-Objekt
  static constexpr int VEXT_PIN = 36; // Heltec-spezifischer Vext-Pin (angepasst)
};

extern Display oled; // Globales Display-Objekt

#endif
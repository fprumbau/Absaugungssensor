#include "display.h"

Display::Display() : oled(OLED_ADDRESS, SDA_OLED, SCL_OLED) {}

bool Display::init() {
  pinMode(VEXT_PIN, OUTPUT);
  pinMode(RST_OLED, OUTPUT);
  vextOn(); // Vext einschalten für Display
  reset();  // Display-Reset durchführen
  Wire.begin(SDA_OLED, SCL_OLED);
  oled.init();
  
  //oled.setBrightness(5); //minimaler Strombezug beim Bootvorgang
  
  Serial.println("OLED init called"); // Debug
  oled.setFont(ArialMT_Plain_10);
  debugPrint(DEBUG_DISPLAY, "Display initialized");
  return true; // Hier könnte eine Fehlerprüfung hinzugefügt werden
}

void Display::reset() {
  digitalWrite(RST_OLED, LOW);  // Reset aktivieren
  delay(10);                   // Längeres Timing
  digitalWrite(RST_OLED, HIGH); // Reset deaktivieren
  delay(10);                   // Wartezeit nach Reset
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

void Display::sendSSD1306Command(uint8_t command) {
  Wire.beginTransmission(0x3C);  // Display-Adresse, passe an falls anders
  Wire.write(0x00);  // Control-Byte für Command-Mode (Co=0, D/C=0)
  Wire.write(command);
  Wire.endTransmission();
}

void Display::setDisplayOrientation(bool flipVertical) {
  if (!flipVertical) {
    oled.resetDisplay();
    oled.resetOrientation();
    debugPrint(DEBUG_DISPLAY, "Display flipped (down)");
    sendSSD1306Command(0xC8);  // Vertical flip (upside down)
    sendSSD1306Command(0xA1); // Für horizontal mirror
  } else {
    oled.resetDisplay();
    oled.resetOrientation();
    debugPrint(DEBUG_DISPLAY, "Display flipped (up)");
    sendSSD1306Command(0xC0);  // Normal vertical
    sendSSD1306Command(0xA0); // Normal horizontal
  }
}

void Display::updateScreen() {
    oled.clear();
    float val = adxl.getGX();
    if(val>0.05) {
      if(flipped) {
        flipped=false;
        setDisplayOrientation(true);
      } 
    } else if (val<-0.05) {
      if(!flipped) {
        flipped=true;
        setDisplayOrientation(false);
      }
    }
    oled.clear();

    if(absaugung.stopped()) {
      long seconds = config.getIdleTime() - (millis() - lastActivityTime) / 1000;
      oled.drawString(0, 0, "Absaug.Sensor       " + String(seconds) + "s");      
    } else {
      oled.drawString(0, 0, "Absaug.Sensor       ---"); 
    }
    oled.drawString(0, 11, "Taster: " + String(TasterState ? "on" : "off"));
    oled.drawString(0, 22, "WiFi: " + String(wifi.isActive() ? wifi.localIP() : "off"));
    oled.drawString(0, 34, "X: " + String(adxl.getGX(), 2) + " g");

    oled.drawRect(91, 22, 29, 16); 
    oled.drawRect(90, 81, 31, 18);
    oled.drawString(95, 24, absaugung.status());

    oled.drawString(0, 42, "Y: " + String(adxl.getGY(), 2) + " g");
    oled.drawString(0, 50, "Z: " + String(adxl.getGZ(), 2) + " g");
    oled.drawString(90, 50, "v.: " + web.version);
    oled.display();
}

Display oled; // Definition
#include "global.h"

void setup() {

    debugLevel = DEBUG_DISPLAY | LORA_MSGS | DEBUG_CONFIG | DEBUG_WIFI; // Debug-Level setzen

    Serial.begin(115200);
    while (!Serial) delay(10);

    if (!oled.init()) {
      Serial.println("Display initialization failed!");
      while (1);
    }
    oled.clear();
    oled.drawString(0, 0, "AbsaugungsSensor startet...");
    oled.display();

    debugPrint(DEBUG_INIT, "AbsaugungsSensor startet...");

    // Taster initialisieren
    pinMode(TASTER_PIN, INPUT_PULLUP);  // Pullup, LOW = gedrückt

    // LoRa initialisieren
    if (!lora.init()) {
      Serial.println("LoRa initialization failed!");
      while (1);
    }

    //ADXL initialisieren
    if (!adxl.init()) {
      debugPrint(DEBUG_ADXL, "ADXL initialization failed!");
      while (1);
    }

    if (!config.load()) {
      debugPrint(DEBUG_CONFIG, "Config load failed, setting and saving new values");
      // Initiale Konfiguration schreiben (einmalig ausführen)
      config.setValue("ssid", "P...y", true);
      config.setValue("pass", "5...7", true); // Speichert sofort mit true
    } 

    // initialize other the air updates
    updater.setup(); 
    // Webserver-Routen
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      wifi.resetTimeout();
      String html = "<html><body><h1>WiFi Config</h1>";
      html += "<form action='/config' method='post'>";
      html += "SSID: <input type='text' name='ssid' value='" + String(config.getSSID()) + "'><br>";
      html += "Pass: <input type='text' name='pass' value='" + String(config.getPass()) + "'><br>";
      html += "<input type='submit' value='Save'></form>";
      html += "<a href='/ota'>OTA Update</a></body></html>";
      request->send(200, "text/html", html);
      debugPrint(DEBUG_WIFI, "Main page accessed");
    });    
}

void loop() {

    if (updater.getUpdating()) {
        updater.loop(); // OTA-Prozess abwickeln
        oled.clear();
        oled.drawString(0, 0, "OTA Update läuft...");
        oled.display();
        return; // Normaler Loop wird unterbrochen
    }  


  bool TasterState = digitalRead(TASTER_PIN) == LOW;

  if (TasterState && !TasterGedrueckt) {
    unsigned long currentTime = millis();
    if (currentTime - TasterPressTime > TasterEntprellZeit) {
      TasterGedrueckt = true;
      TasterPressTime = currentTime;
      debugPrint(DEBUG_DISPLAY, "Taster gedrückt");
    }
  } else if (TasterState && TasterGedrueckt) {
    unsigned long pressDuration = millis() - TasterPressTime;
    if (pressDuration > WifiActivationTime && !wifi.isActive()) {
      debugPrint(DEBUG_WIFI, "Starting WiFi with SSID=" + String(config.getSSID()) + ", Pass=" + String(config.getPass()));
      wifi.begin(config.getSSID(), config.getPass());
      oled.clear();
      oled.drawString(0, 0, "WiFi-Modus aktiviert");
      oled.display();
    }
  } else if (!TasterState && TasterGedrueckt) {
    TasterGedrueckt = false;
    debugPrint(DEBUG_DISPLAY, "Taster losgelassen");
  }

    wifi.loop(); // Prüft Timeout

    adxl.update();
    if (adxl.detectMovement(0.2)) {
      debugPrint(DEBUG_ADXL, "Bewegung erkannt");
    }

    oled.clear();
    oled.drawString(0, 0, "AbsaugungsSensor");
    oled.drawString(0, 16, "Taster: " + String(TasterState ? "gedrückt" : "los"));
    oled.drawString(0, 32, "WiFi: " + String(wifi.isActive() ? "aktiv" : "inaktiv"));
    oled.drawString(0, 40, "X: " + String(adxl.getGX(), 2) + " g");
    oled.drawString(0, 48, "Y: " + String(adxl.getGY(), 2) + " g");
    oled.drawString(0, 56, "Z: " + String(adxl.getGZ(), 2) + " g");
    oled.display();

    static bool lastTasterState = false;
    if (TasterState != lastTasterState || adxl.detectMovement(0.2)) {
      String message = "Taster: " + String(TasterState ? "gedrückt" : "los") +
                      ", X: " + String(adxl.getGX(), 2) + " g" +
                      ", Y: " + String(adxl.getGY(), 2) + " g" +
                      ", Z: " + String(adxl.getGZ(), 2) + " g";
      lora.send(message, 1000, 10);
      lastTasterState = TasterState;
    }

    delay(500);

}
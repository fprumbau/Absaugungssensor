#include "global.h"

void setup() {
    debugLevel = DEBUG_DISPLAY | LORA_MSGS | DEBUG_CONFIG | DEBUG_WIFI;

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

    pinMode(TASTER_PIN, INPUT_PULLUP);

    if (!lora.init()) {
      Serial.println("LoRa initialization failed!");
      while (1);
    }

    if (!adxl.init()) {
      debugPrint(DEBUG_ADXL, "ADXL initialization failed!");
      while (1);
    }

    if (!config.load()) {
      debugPrint(DEBUG_CONFIG, "Config load failed, setting and saving new values");
      config.setValue("ssid", "P...y", true);
      config.setValue("pass", "5...7", true);
    }

    updater.setup();
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
        updater.loop();
        oled.clear();
        oled.drawString(0, 0, "OTA Update läuft...");
        oled.display();
        return;
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
        lora.send("Testnachricht", 1000, 10); // Bestehende String-Version
    }

    wifi.loop();

    adxl.update();
    if (adxl.detectMovement(0.2)) {
        debugPrint(DEBUG_ADXL, "Bewegung erkannt");
        // Teste neues Protokoll mit einer einfachen Aktion
        lora.send(1); // Sende "starte"
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
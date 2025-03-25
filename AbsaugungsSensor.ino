#include "global.h"

void setup() {
    debugLevel = LORA_MSGS | DEBUG_CONFIG | DEBUG_ADXL | DEBUG_WIFI;

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
      html += "Idle Time (s): <input type='number' name='idleTime' value='" + String(config.getIdleTime()) + "'><br>";
      html += "<input type='submit' value='Save'></form>";
      html += "<a href='/ota'>OTA Update</a></body></html>";
      request->send(200, "text/html", html);
      debugPrint(DEBUG_WIFI, "Main page accessed");
    });

    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("ssid", true)) {
        config.setValue("ssid", request->getParam("ssid", true)->value(), false);
      }
      if (request->hasParam("pass", true)) {
        config.setValue("pass", request->getParam("pass", true)->value(), false);
      }
      if (request->hasParam("idleTime", true)) {
        config.setValue("idleTime", request->getParam("idleTime", true)->value(), false);
      }
      config.save();
      request->send(200, "text/html", "<html><body><h1>Config saved</h1><a href='/'>Back</a></body></html>");
      debugPrint(DEBUG_WIFI, "Config updated via web");
    });
}

void loop() {
    static unsigned long lastActivityTime = millis();
    static bool sleepAnnounced = false;

    if (updater.getUpdating()) {
        updater.loop();
        oled.clear();
        oled.drawString(0, 0, "OTA Update läuft...");
        oled.display();
        lastActivityTime = millis(); // Aktivität zurücksetzen
        return;
    }

    bool TasterState = digitalRead(TASTER_PIN) == LOW;

    // Tasterlogik mit Entprellung
    if (TasterState && !TasterGedrueckt) {
        unsigned long currentTime = millis();
        if (currentTime - TasterPressTime > TasterEntprellZeit) {
            TasterGedrueckt = true;
            TasterPressTime = currentTime;
            debugPrint(DEBUG_DISPLAY, "Taster gedrückt");
            lastActivityTime = currentTime;
        }
    } else if (TasterState && TasterGedrueckt) {
        unsigned long pressDuration = millis() - TasterPressTime;
        if (pressDuration > WifiActivationTime && !wifi.isActive()) {
            debugPrint(DEBUG_WIFI, "Starting WiFi with SSID=" + String(config.getSSID()) + ", Pass=" + String(config.getPass()));
            wifi.begin(config.getSSID(), config.getPass());
            oled.clear();
            oled.drawString(0, 0, "WiFi-Modus aktiviert");
            oled.display();
            lastActivityTime = millis();
        }
    } else if (!TasterState && TasterGedrueckt) {
        TasterGedrueckt = false;
        debugPrint(DEBUG_DISPLAY, "Taster losgelassen");
        lora.send("Testnachricht", 1000, 10);
        lastActivityTime = millis();
    }


    wifi.loop();

    adxl.readAccelerometer();
    bool vibrationDetected = adxl.detectMovement(0.2);

    static bool absaugungAktiv = false;
    static unsigned long lastSendTime = 0;
    static bool awaitingConfirmation = false;
    static uint8_t lastAction = 0;

    if (vibrationDetected && !absaugungAktiv && !awaitingConfirmation) {
        awaitingConfirmation = true;
        lastAction = 1; // Start
        if (lora.send(lastAction)) {
            lastSendTime = millis();
            debugPrint(LORA_MSGS, "Vibration detected, start sent");
            lastActivityTime = millis(); // Aktivität zurücksetzen
        } else {
            debugPrint(LORA_MSGS, "Vibration detected, start send failed");
            awaitingConfirmation = false;
        }
    } else if (!vibrationDetected && absaugungAktiv && !awaitingConfirmation) {
        awaitingConfirmation = true;
        lastAction = 2; // Stop
        if (lora.send(lastAction)) {
            lastSendTime = millis();
            debugPrint(LORA_MSGS, "Vibration stopped, stop sent");
            lastActivityTime = millis(); // Aktivität zurücksetzen
        } else {
            debugPrint(LORA_MSGS, "Vibration stopped, stop send failed");
            awaitingConfirmation = false;
        }
    }

    if (awaitingConfirmation && (millis() - lastSendTime >= 3000)) {
        if (lora.send(lastAction)) {
            lastSendTime = millis();
            debugPrint(LORA_MSGS, "Retry sent: sensor" + String(SENSOR_ID) + ": " + String(lora.actionToString(lastAction)));
            lastActivityTime = millis(); // Aktivität zurücksetzen
        } else {
            debugPrint(LORA_MSGS, "Retry failed: sensor" + String(SENSOR_ID) + ": " + String(lora.actionToString(lastAction)));
        }
    }


    // Neue Empfangslogik
    uint8_t sensorId, action;
    if (lora.receive(sensorId, action)) {
        if (sensorId == SENSOR_ID) {
            if (action == 3 && lastAction == 1) { // "started" empfangen
                absaugungAktiv = true;
                awaitingConfirmation = false;
                debugPrint(LORA_MSGS, "Received confirmation: sensor" + String(SENSOR_ID) + ": started");
                lastActivityTime = millis(); // Aktivität zurücksetzen
            } else if (action == 4 && lastAction == 2) { // "gestoppt" empfangen
                absaugungAktiv = false;
                awaitingConfirmation = false;
                debugPrint(LORA_MSGS, "Received confirmation: sensor" + String(SENSOR_ID) + ": gestoppt");
                lastActivityTime = millis(); // Aktivität zurücksetzen
            }
        }
    }

    // Tiefschlaf-Logik
    if (!absaugungAktiv && !wifi.isActive() && (millis() - lastActivityTime >= config.getIdleTime() * 1000)) {
        if (!sleepAnnounced) {
            debugPrint(DEBUG_INIT, "Entering deep sleep in 3 seconds...");
            oled.clear();
            oled.drawString(0, 0, "Deep Sleep in 3s");
            oled.display();
            unsigned long sleepAnnounceStart = millis();
            while (millis() - sleepAnnounceStart < 3000) {
                if (digitalRead(TASTER_PIN) == LOW) {
                    debugPrint(DEBUG_INIT, "Deep sleep aborted by taster");
                    oled.clear();
                    oled.drawString(0, 0, "Sleep aborted");
                    oled.display();
                    delay(1000); // Kurz anzeigen
                    sleepAnnounced = false;
                    lastActivityTime = millis();
                    return;
                }
                delay(10); // Kurze Pausen für Reaktivität
            }
            sleepAnnounced = true;
        }
        Serial.println("Going to deep sleep now...");
        Serial.println("GPIO 0 state before sleep: " + String(digitalRead(TASTER_PIN)));
        adxl.sleep(); // ADXL ausschalten

        if (digitalRead(TASTER_PIN) == 0) {
            Serial.println("Error: GPIO 0 is LOW, cannot enter deep sleep safely");
            while (1);
        }
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // Aufwecken bei LOW
        esp_deep_sleep_start();
    } else {
        sleepAnnounced = false;
    }

    oled.clear();
    oled.drawString(0, 0, "AbsaugungsSensor");
    oled.drawString(0, 13, "Taster: " + String(TasterState ? "gedrückt" : "los"));
    oled.drawString(0, 26, "WiFi: " + String(wifi.isActive() ? "aktiv" : "inaktiv"));
    oled.drawString(0, 34, "X: " + String(adxl.getGX(), 2) + " g");
    oled.drawString(0, 42, "Y: " + String(adxl.getGY(), 2) + " g");
    oled.drawString(0, 50, "Z: " + String(adxl.getGZ(), 2) + " g");
    oled.display();

    static bool lastTasterState = false;
    if (TasterState != lastTasterState || vibrationDetected) {
        String message = "Taster: " + String(TasterState ? "gedrückt" : "los") +
                         ", X: " + String(adxl.getGX(), 2) + " g" +
                         ", Y: " + String(adxl.getGY(), 2) + " g" +
                         ", Z: " + String(adxl.getGZ(), 2) + " g";
        //lora.send(message, 1000, 10);
        debugPrint(DEBUG_ADXL, message);
        lastTasterState = TasterState;
        lastActivityTime = millis(); // Aktivität zurücksetzen
    }

    delay(100);
}
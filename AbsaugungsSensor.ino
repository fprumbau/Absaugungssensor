#include "global.h"

/**
* Board: Heltec LoRa(v3) in der Liste der Esp32-Boards auswaehlen!!!
*/

void setup() {
    WiFi.mode(WIFI_OFF);
    delay(50);

    debugLevel = LORA_MSGS | DEBUG_CONFIG | DEBUG_WIFI | DEBUG_ABSG | DEBUG_SWITCH;

    Serial.begin(115200);
    while (!Serial) delay(10);

    if (!oled.init()) {
      Serial.println("Display initialization failed!");
      while (100);
    }
    oled.clear();
    oled.drawString(0, 0, "AbsaugungsSensor startet...");
    oled.display();

    debugPrint(DEBUG_INIT, "AbsaugungsSensor startet...");

    pinMode(TASTER_PIN, INPUT_PULLUP);

    absaugung.init();

    oled.clear();
    oled.drawString(0, 0, "ADXL startet...");
    oled.display();

    //if (!adxl.init()) {
    if(!vibrationSensor.begin()) {    
        oled.clear();
        oled.drawString(0, 0, "ADXL initialization failed!");
        oled.display();
        debugPrint(DEBUG_ADXL, "ADXL initialization failed!");
        while (1);
    }
    
    oled.clear();
    oled.drawString(0, 0, "Config load...");
    oled.display();
    if (!config.load()) {
      debugPrint(DEBUG_CONFIG, "Config load failed, setting and saving new values");
      config.setValue("ssid", "P...y", true);
      config.setValue("pass", "5...7", true);
    }
    
    oled.clear();
    oled.drawString(0, 0, "Web setup...");
    oled.display();

    web.setup();

}

void loop() {

    //return;

    //Restart erforderlich, wird durch updater-Objekt nach Upload einer neuen Firmware geregelt
    if(web.restartRequired) {
      delay(2000);
      ESP.restart();
    }
    if (web.getUpdating()) {
        web.loop();
        oled.clear();
        oled.drawString(0, 0, "OTA Update läuft...");
        oled.display();
        lastActivityTime = millis(); // Aktivität zurücksetzen
        return;
    }

    TasterState = digitalRead(TASTER_PIN) == LOW;

    // Tasterlogik mit Entprellung
    if (TasterState && !TasterGedrueckt) {
        unsigned long currentTime = millis();
        if (currentTime - TasterPressTime > TasterEntprellZeit) {
            TasterGedrueckt = true;
            TasterPressTime = currentTime;
            lastActivityTime = currentTime;
            //Status abfragen
            debugPrint(LORA_MSGS, "\nQuery current state for this sensor " + String(SENSOR_ID));
            lora.send(QUERY);
        }
    } else if (TasterState && TasterGedrueckt) {
        unsigned long pressDuration = millis() - TasterPressTime;
        if (pressDuration > WifiActivationTime && !wifi.isActive()) {
            debugPrint(DEBUG_ABSG, "Starting WiFi with SSID=" + String(config.getSSID()) + ", Pass=" + String(config.getPass()));
            wifi.begin(config.getSSID(), config.getPass());
            oled.clear();
            oled.drawString(0, 0, "WiFi-Modus aktiv: " + WiFi.localIP().toString());
            oled.display();
            lastActivityTime = millis();
        } 
    } else if (!TasterState && TasterGedrueckt) { //Schalter wurde gerade wieder geloest
        unsigned long pressDuration = millis() - TasterPressTime;
        TasterGedrueckt = false;
        debugPrint(DEBUG_DISPLAY, "Taster losgelassen");
        lastActivityTime = millis();
        if(pressDuration < maxShortPressTime) {
            debugPrint(DEBUG_SWITCH, "\nSchalte Absaugung um");
            absaugung.toggle();
        }/* else {
            debugPrint(DEBUG_SWITCH, "Schalter wurde <maxShortPressTime betaetigt, ignoriere...");
        }*/
    } 

    wifi.loop();
    absaugung.loop();

    //adxl.readAccelerometer();
    //bool vibrationDetected = adxl.detectMovement(0.2);

    /*if (vibrationDetected 
                  && absaugung.stopped() 
                  && !absaugung.awaitsConfirmation()) {
        if (absaugung.start()) {
            debugPrint(LORA_MSGS, "Vibration detected, start sent");
            lastActivityTime = millis(); // Aktivität zurücksetzen
        } else {
            debugPrint(LORA_MSGS, "Vibration detected, start send failed");
        }
    } else if (!vibrationDetected 
                  && absaugung.started() 
                  && !absaugung.awaitsConfirmation()) {
        if (absaugung.stop()) {;
            debugPrint(LORA_MSGS, "Vibration stopped, stop sent");
            lastActivityTime = millis(); // Aktivität zurücksetzen
        } else {
            debugPrint(LORA_MSGS, "Vibration stopped, stop send failed");
        }
    }*/

    /*
    //Bestaetigung noch nicht erfolgt
    if (awaitingConfirmation && (millis() - lastSendTime >= 3000)) {
        if (lora.send(lastAction)) {
            lastSendTime = millis();
            debugPrint(LORA_MSGS, "Retry sent: sensor" + String(SENSOR_ID) + ": " + String(lora.actionToString(lastAction)));
            lastActivityTime = millis(); // Aktivität zurücksetzen
        } else {
            debugPrint(LORA_MSGS, "Retry failed: sensor" + String(SENSOR_ID) + ": " + String(lora.actionToString(lastAction)));
        }
    }*/

    // Tiefschlaf-Logik
    if (absaugung.stopped() && !wifi.isActive() && (millis() - lastActivityTime >= config.getIdleTime() * 1000)) {
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
        //adxl.sleep(); // ADXL ausschalten

        if (digitalRead(TASTER_PIN) == 0) {
            Serial.println("Error: GPIO 0 is LOW, cannot enter deep sleep safely");
            while (1);
        }
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // Aufwecken bei LOW
        esp_deep_sleep_start();
    } else {
        sleepAnnounced = false;
    }

    /*static bool lastTasterState = false;
    if (TasterState != lastTasterState || vibrationDetected) {
        String message = "Taster: " + String(TasterState ? "on" : "off") +
                         ", X: " + String(adxl.getGX(), 2) + " g" +
                         ", Y: " + String(adxl.getGY(), 2) + " g" +
                         ", Z: " + String(adxl.getGZ(), 2) + " g";
        //lora.send(message, 1000, 10);
        debugPrint(DEBUG_ADXL, message);
        lastTasterState = TasterState;
        lastActivityTime = millis(); // Aktivität zurücksetzen
    }*/

    oled.updateScreen();

    //delay(100);
}
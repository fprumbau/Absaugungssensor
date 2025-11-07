#include "global.h"

/**
* Board: Heltec LoRa(v3) in der Liste der Esp32-Boards auswaehlen!!!
*/

void setup() {
    WiFi.mode(WIFI_OFF);
    delay(50);

    debugLevel = DEBUG_ADXL;

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

    bool adxlInit = adxl.init();
    if (!adxlInit) {   
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
        }
    } 

    wifi.loop();
    absaugung.loop();

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

    oled.updateScreen();

    commandLine();
}

void commandLine() {
  if(Serial.available()) {
      String cmd = Serial.readStringUntil('\n'); 
      Serial.print(F("Echo: "));
      Serial.println(cmd);
      String msg = String((char*)0);
      msg.reserve(32);
      if(cmd.startsWith(F("restart wifi"))) {      
        //myWifi.reconnect();
      } else if(cmd.startsWith(F("restart esp"))) {      
        msg = F("Restarting ESP...");
        Serial.println(msg);
        ESP.restart();
      } else if(cmd.startsWith(F("show heap"))) {
        Serial.print(F("Free heap: "));
        Serial.println(ESP.getFreeHeap()); 
      } else if(cmd.startsWith(F("config load"))) {
        config.load();
      } else if(cmd.startsWith(F("config save"))) {
        config.save();
      }  else if(cmd.startsWith(F("config set"))) {
        String keyVal = cmd.substring(10); //alles hinter 'set'
        keyVal.trim();
        //config.set(keyVal);
      } else if(cmd.startsWith(F("config persist"))) {
        String keyVal = cmd.substring(14); //alles hinter 'persist'
        keyVal.trim();
        //String key = config.getValue(keyVal, ':', 0);
        //String val = config.getValue(keyVal, ':', 1);
        //config.save(key,val);
      } else if(cmd.startsWith(F("adxl learn"))) {
        String werkzeug = cmd.substring(10);
        adxl.learn(werkzeug.c_str());
      } else if(cmd.startsWith(F("adxl save"))) {
        adxl.saveProfile();
      } else if(cmd.startsWith(F("adxl print"))) {
        adxl.printProfile();
      } else if(cmd.startsWith(F("config show"))) {
        String key = cmd.substring(11); //alles hinter 'show'
        const char* val = config.load(key);
        Serial.println(val);
      } else if(cmd.startsWith(F("cmd"))) {  
        String nrStr = cmd.substring(3);
        nrStr.trim();
        int nr = nrStr.toInt();
        switch(nr) {
          case 0:
            Serial.println(F("serialSBMS.flush();"));
            break;
          case 1:
            Serial.println(F("Serial.println(serialSBMS.available());"));
            break;             
          default:
            Serial.println("Kein Kommando mit dieser Nummer gefunden");
        }
        
      } else if(cmd.startsWith(F("test wifi"))) { 
        bool ok = WiFi.status() == WL_CONNECTED;
        Serial.print(F("Wifi status: "));
        Serial.println(ok);
      } else if(cmd.startsWith(F("verbose"))) { 
        esp_log_level_set("*", ESP_LOG_VERBOSE);
      } else {
        Serial.println(F("Available commands:"));
        Serial.println(F(" - restart wifi  :: restarting Wifi connection"));
        Serial.println(F(" - restart esp   :: restarting whole ESP32"));
        Serial.println(F(" - test  on|off :: enable/disable test simulation"));
        Serial.println(F(" - debug  on|off :: enable/disable debug"));        
        Serial.println(F(" - cmd NR :: Kommando mit der u.a. Nummer ausfuehren"));
        Serial.println(F(" -      0 :: serialSBMS.flush();"));
        Serial.println(F(" -      1 :: Serial.println(serialSBMS.available());"));       
        Serial.println(F(" - adxl learn :: Lernmodus an"));
        Serial.println(F(" - config load|save :: Schreiben/Lesen der Konfig aus LITTLEFS"));
        Serial.println(F(" - config set key:value :: Hinzufuegen/aendern eines Konfigwertes (ohne Speichern!), z.B. socLimit"));
        Serial.println(F(" - config persist key:value :: Speichern/aendern eines Konfigwertes (mit Speichern!), z.B. socLimit"));        
        Serial.println(F(" - config show key :: Ausgabe des gespeicherten Values von 'key' auf Serial"));
        Serial.println(F(" - show heap :: Schreibe den noch verfuegbaren Heap in die Ausgabe"));
        Serial.println(F(" - test wifi :: Verbindungsstatus von Wifi ausgeben"));
        Serial.println(F(" - verbose :: Aktiviert ESP verbose logging ( esp_log_level_set('*', ESP_LOG_VERBOSE) )"));
        Serial.println(F(" - print :: Schreibe einige abgeleitete Werte auf den Bildschirm"));     
        return;
      }


      Serial.println(msg); 
  }
}
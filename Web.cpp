#include "CFG.h"
#include "Web.h"
#include "Update.h"

Web::Web() : isUpdating(false) {}

void Web::setup() {
    version = "0.14";
    server.on("/ota", HTTP_GET, [this](AsyncWebServerRequest *request){
        wifi.resetTimeout();
        String html = "<html><body><h1>OTA Update</h1>";
        html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
        html += "<input type='file' name='update'><br>";
        html += "<input type='submit' value='Update'></form>";
        html += "<h2>Changelog</h2>";
        html += "<ul>";
        html += "<li><b>0.1</b> 2025-03-25: Initial version with OTA support</li>";
        html += "<li><b>0.2</b> 2025-03-26: Added WiFi activation</li>";
        html += "<li><b>0.3</b> 2025-03-29: Absaugung als Kommunikationsobjekt</li>";    
        html += "<li><b>0.4</b> 2025-03-30: SensorID in Konfig</li>";   
        html += "<li><b>0.5</b> 2025-10-25: Das Display rotiert, wenn man das Ger&auml;t dreht</li>";  
        html += "<li><b>0.6</b> 2025-10-26: Nach einem OTA muss ein Restart gemacht werden.</li>";    
        html += "<li><b>0.7</b> 2025-10-26: Off/On-Anzeige der Absaugung im Display.</li>";   
        html += "<li><b>0.8</b> 2025-10-26: Implementierung ACK, Anzeige Restidlezeit.</li>"; 
        html += "<li><b>0.9</b> 2025-11-02: Hysterese von 20 bei Displayflip.</li>";         
        html += "<li>                       Absaugungstoggle bei Klick<1s.</li>";      
        html += "<li><b>0.10</b> 2025-11-03: ADXL lernt.</li>";  
        html += "<li><b>0.11</b> 2025-11-04:VerbindungsSensor nach ADXL konsolidiert.</li>";      
        html += "<li>                       Profildaten lad- und speicherbar.</li>";   
        html += "<li><b>0.12</b> 2025-11-05:Kommandozeilenmodus hinzugefuegt.</li>";        
        html += "<li><b>0.13</b> 2025-11-07:Profilspeicherung in CFG.</li>";  
        html += "<li><b>0.14</b> 2025-11-16:Fix Displayorientation.</li>"; 
        html += "</ul>";
        html += "<a href='/'>Back</a></body></html>";
        request->send(200, "text/html", html);
        debugPrint(DEBUG_WIFI, "OTA page accessed");
    });
    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
        isUpdating = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(
            Update.hasError() ? 500 : 200,
            "text/plain",
            Update.hasError() ? "FAIL" : "OK"
        );
        response->addHeader("Connection", "close");
        request->send(response);
        debugPrint(DEBUG_WIFI, Update.hasError() ? "OTA Update failed" : "OTA Update succeeded");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.println("Update Start: " + filename);
            Update.begin((ESP.getFreeSketchSpace() - (ESP.getFreeSketchSpace() % 4096)) / 4096 * 4096);
        }
        if (!Update.hasError()) {
            Update.write(data, len);
        }
        if (final) {
            if (Update.end(true)) { 
                Serial.println("Update Success");
            } else {
                Serial.println("Update Failed");
            }
            web.restartRequired = true;  // Tell the main loop to restart the ESP
        }
    });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      wifi.resetTimeout();
      String html = "<html><body><h1>WiFi Config</h1>";
      html += "<form action='/config' method='post'>";
      html += "SensorID: <input type='text' name='sensorId' value='" + String(SENSOR_ID) + "'><br>";      
      html += "SSID: <input type='text' name='ssid' value='" + String(config.getSSID()) + "'><br>";
      html += "Pass: <input type='text' name='pass' value='" + String(config.getPass()) + "'><br>";
      html += "Idle Time (s): <input type='number' name='idleTime' value='" + String(config.getIdleTime()) + "'><br>";
      html += "Profile: <input type='text' name='profile' value='" + String(config.getProfile()) + "'><br>";
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
      if (request->hasParam("sensorId", true)) {
        config.setValue("sensorId", request->getParam("sensorId", true)->value(), false);
      }      
      if (request->hasParam("profile", true)) {
        config.setValue("profile", request->getParam("profile", true)->value(), false);
      }    
      config.save();
      request->send(200, "text/html", "<html><body><h1>Config saved</h1><a href='/'>Back</a></body></html>");
      debugPrint(DEBUG_WIFI, "Config updated via web");
    });    
}

void Web::loop() {
    if (isUpdating) {
        Serial.println("OTA Update in progress...");
    }
}

bool Web::getUpdating() {
    return isUpdating;
}

Web web;
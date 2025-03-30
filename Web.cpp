#include "CFG.h"
#include "Web.h"
#include "Update.h"

Web::Web() : isUpdating(false) {}

void Web::setup() {
    server.on("/ota", HTTP_GET, [this](AsyncWebServerRequest *request){
        wifi.resetTimeout();
        String html = "<html><body><h1>OTA Update</h1>";
        html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
        html += "<input type='file' name='update'><br>";
        html += "<input type='submit' value='Update'></form>";
        html += "<h2>Changelog</h2>";
        html += "<ul>";
        html += "<li>2025-03-25: Initial version with OTA support</li>";
        html += "<li>2025-03-26: Added WiFi activation</li>";
        html += "<li>2025-03-29: Absaugung als Kommunikationsobjekt</li>";    
        html += "<li>2025-03-30: SensorID in Konfig</li>";              
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
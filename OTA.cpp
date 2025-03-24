#include "OTA.h"
#include "Update.h"

OTA::OTA() : isUpdating(false) {}

void OTA::setup() {
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
}

void OTA::loop() {
    if (isUpdating) {
        Serial.println("OTA Update in progress...");
    }
}

bool OTA::getUpdating() {
    return isUpdating;
}

OTA updater;
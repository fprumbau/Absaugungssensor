#include "MyWifi.h"

MyWifi::MyWifi() : active(false), startTime(0) {}

bool MyWifi::begin(const char* ssid, const char* password) {
  this->ssid = ssid;
  this->password = password;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long connectStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - connectStart < 10000) {
    delay(500);
    debugPrint(DEBUG_DISPLAY, "Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    active = true;
    startTime = millis();
    debugPrint(DEBUG_DISPLAY, "WiFi connected: " + String(ssid));
    return true;
  } else {
    Serial.println("WiFi connection failed");
    WiFi.disconnect();
    return false;
  }
}

void MyWifi::loop() {
  if (!active) return;

  if (millis() - startTime > TIMEOUT_MS) {
    disconnect();
    debugPrint(DEBUG_DISPLAY, "WiFi timeout reached");
  }
}

bool MyWifi::isConnected() {
  return active && WiFi.status() == WL_CONNECTED;
}

void MyWifi::disconnect() {
  WiFi.disconnect();
  active = false;
  debugPrint(DEBUG_DISPLAY, "WiFi disconnected");
}

bool MyWifi::isActive() const {
  return active;
}

void MyWifi::resetTimeout() {
  if (active) {
    startTime = millis();
    debugPrint(DEBUG_DISPLAY, "WiFi timeout reset");
  }
}

MyWifi wifi;
const long WifiActivationTime = 6000; // 6 Sekunden
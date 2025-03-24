#ifndef MYWIFI_H
#define MYWIFI_H

#include "global.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class MyWifi {
public:
  MyWifi(AsyncWebServer& server); // Server als Referenz übergeben
  bool begin(const char* ssid, const char* password);
  void loop();
  bool isConnected();
  void disconnect();
  bool isActive() const;
  void resetTimeout();

private:
  String ssid;
  String password;
  bool active;
  unsigned long startTime;
  AsyncWebServer& server; // Referenz auf den Webserver
  static constexpr unsigned long TIMEOUT_MS = 120000; // 120 Sekunden
};

extern MyWifi wifi;
extern const long WifiActivationTime; 

#endif
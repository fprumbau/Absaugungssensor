#ifndef MYWIFI_H
#define MYWIFI_H

#include <WiFi.h>

class MyWifi {
    private:
      String _ip;
      IPAddress _localIP;
      long lastReconnectMillis;
      const int myWifiRestartLimit = 50;
      boolean isBooting = true;
    public:
      int wifiReconnects = 0; //0.9.9.92 Zaehlen von Wifi Reconnects
      void connect();
      String getIpAddress();
      IPAddress localIP();
      void reconnect();
      bool connected();
      void print();
};

#endif
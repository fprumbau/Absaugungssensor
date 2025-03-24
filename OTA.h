#ifndef OTA_H
#define OTA_H

#include "global.h"

class OTA {
private:
    bool isUpdating;

public:
    OTA();
    void setup();
    void loop();
    bool getUpdating();
};

extern OTA updater;

#endif
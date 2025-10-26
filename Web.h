#ifndef WEB_H
#define WEB_H

#include "global.h"

class Web {
private:
    bool isUpdating;

public:
    String version;
    bool restartRequired = false; 
    Web();
    void setup();
    void loop();
    bool getUpdating();
};

extern Web web;

#endif
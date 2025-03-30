#ifndef ABSAUGUNG_H
#define ABSAUGUNG_H

#include "global.h"

class Absaugung {
    public:
        void init();
        bool start();
        bool stop(); //beruecksichtigt Nachlauf
        void toggle();
        bool started();
        bool stopped();
        void loop();
        bool awaitsConfirmation();
    private:
        bool awaitingConfirmation = false;
        unsigned long lastSendTime = 0;
        bool isStarted = false;
        bool startWaiting = false;
        bool stopWaiting = false;
}; 

extern Absaugung absaugung;

#endif
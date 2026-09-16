#pragma once

#include <Arduino.h>
#include "config.h"
#include "esp_sleep.h"

class App;

class PowerManager {
public:
    explicit PowerManager(App* app) : _app(app) {}

    void sleepDisplay();

    void wakeDisplay();

    esp_sleep_wakeup_cause_t enterLightSleep();

    bool isDisplayOn() const { return _displayOn; }

    bool isSleeping() const { return _sleeping; }

    void resetInactivityTimer();

    bool isInactivityTimeout() const;

    unsigned long lastActivityMs() const { return _lastActivity; }

private:
    App* _app;
    bool  _displayOn = true;
    bool  _sleeping = false;
    unsigned long _lastActivity = 0;
};

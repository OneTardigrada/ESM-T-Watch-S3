#pragma once

#include <Arduino.h>
#include "config.h"

class App;

class SerialProtocol {
public:
    explicit SerialProtocol(App* app) : _app(app) {}

    void process();

private:
    App*           _app;
    bool           _authenticated = false;
    unsigned long  _authTime = 0;

    void handleCommand(const String& cmd);
    void cmdAuth(const String& token);
    void cmdExport();
    void cmdConfig();
    void cmdConfigGet();
    void cmdDelete();
    void cmdStatus();
    void cmdInfo();
    void cmdHelp();
    void cmdSelfTest();

    bool checkAuth();
    void resetSession();
};

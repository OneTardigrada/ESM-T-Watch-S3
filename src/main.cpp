// main.cpp - ESP32 Entry Point
#include "App.h"

static App app;

void setup() {
    app.setup();
}

void loop() {
    app.loop();
}
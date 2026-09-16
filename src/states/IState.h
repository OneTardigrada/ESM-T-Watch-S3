#pragma once

#include <Arduino.h>

class App;

enum class StateType : uint8_t {
    Home,
    Settings,
    LockScreen,
    Questionnaire,
    Notification,
    Done,
    Status
};

class IState {
public:
    explicit IState(App* app) : _app(app) {}
    virtual ~IState() = default;

    virtual void enter() = 0;

    virtual void update() = 0;

    virtual void exit() = 0;

    virtual StateType type() const = 0;

protected:
    App* _app;
};

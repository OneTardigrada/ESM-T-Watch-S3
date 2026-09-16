#pragma once
#include "states/IState.h"
#include <lvgl.h>

class DoneState : public IState {
public:
    DoneState(App* app, const char* message)
        : IState(app)
    {
        strncpy(_message, message, sizeof(_message) - 1);
    }

    void enter() override;
    void update() override;
    void exit() override;
    StateType type() const override { return StateType::Done; }

private:
    char _message[128] = {};
    static void okCb(lv_event_t* e);
};

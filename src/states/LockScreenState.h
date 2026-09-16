#pragma once
#include "states/IState.h"
#include <lvgl.h>

class LockScreenState : public IState {
public:
    enum class Action : uint8_t { UnlockSettings, UnlockDelete };

    LockScreenState(App* app, Action action)
        : IState(app), _action(action) {}

    void enter() override;
    void update() override;
    void exit() override;
    StateType type() const override { return StateType::LockScreen; }

private:
    Action _action;
    String _pinBuffer;
    bool _showPin = false;
    lv_obj_t* _displayLbl = nullptr;
    lv_obj_t* _eyeLbl = nullptr;

    static void digitCb(lv_event_t* e);
    static void clearCb(lv_event_t* e);
    static void cancelCb(lv_event_t* e);
    static void eyeCb(lv_event_t* e);
    void onDigit(uint8_t digit);
    void checkPin();
    void updateDisplay();
};

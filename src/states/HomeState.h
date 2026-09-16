#pragma once
#include "states/IState.h"
#include <lvgl.h>

class HomeState : public IState {
public:
    explicit HomeState(App* app) : IState(app) {}
    void enter() override;
    void update() override;
    void exit() override;
    StateType type() const override { return StateType::Home; }

private:
    lv_obj_t* _timeLbl = nullptr;
    lv_obj_t* _dateLbl = nullptr;
    lv_obj_t* _battLbl = nullptr;
    lv_obj_t* _battIcon = nullptr;
    lv_obj_t* _alarmIcon = nullptr;
    lv_obj_t* _usbIcon = nullptr;

    int8_t   _lastHour = -1;
    int8_t   _lastMin  = -1;
    int8_t   _lastWday = -1;
    int8_t   _lastMday = -1;
    int8_t   _lastMon  = -1;
    int16_t  _lastBatt = -1;
    int8_t   _lastChg  = -1;
    int8_t   _lastUsbMode = -1;
    int8_t   _lastAlarmVisible = -1;

    static void settingsCb(lv_event_t* e);
    static void cravingCb(lv_event_t* e);
    static void alarmIconCb(lv_event_t* e);
    void updateBattIcon(int bp, bool chg);
    void updateAlarmBanner();
};

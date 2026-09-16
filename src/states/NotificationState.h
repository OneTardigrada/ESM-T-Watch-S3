#pragma once
#include "states/IState.h"
#include "config.h"
#include <lvgl.h>

class NotificationState : public IState {
public:
    NotificationState(App* app, const char* questName, int8_t alarmIdx)
        : IState(app), _alarmIdx(alarmIdx)
    {
        strncpy(_questName, questName, sizeof(_questName) - 1);
    }

    void enter() override;
    void update() override;
    void exit() override;
    StateType type() const override { return StateType::Notification; }

private:
    char _questName[MAX_NAME_LEN] = {};
    int8_t _alarmIdx;

    static void acceptCb(lv_event_t* e);
    static void declineCb(lv_event_t* e);
};

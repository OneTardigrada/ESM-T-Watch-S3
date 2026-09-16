#pragma once
#include "states/IState.h"
#include <lvgl.h>

class SettingsState : public IState {
public:
    explicit SettingsState(App* app) : IState(app) {}
    void enter() override;
    void update() override;
    void exit() override;
    StateType type() const override { return StateType::Settings; }

private:
    static void btnCb(lv_event_t* e);
    void onButton(uint8_t action);

    lv_obj_t* _hourLbl = nullptr;
    lv_obj_t* _minLbl = nullptr;
    lv_obj_t* _dayLbl = nullptr;
    lv_obj_t* _monLbl = nullptr;
    lv_obj_t* _yearLbl = nullptr;
    lv_obj_t* _counterLbl = nullptr;
    lv_obj_t* _brightLbl = nullptr;
    lv_obj_t* _brightSlider = nullptr;
    uint8_t _editH = 0, _editM = 0;
    uint8_t _editD = 1, _editMo = 1;
    uint16_t _editY = 2026;
    uint8_t _editBright = 200;
    bool _timeEditing = false;
    bool _dateEditing = false;

    void showMainMenu();
    void showTimeEditor();
    void showDateEditor();
    void showBrightnessEditor();

    static void timeUpCb(lv_event_t* e);
    static void timeDownCb(lv_event_t* e);
    static void timeSaveCb(lv_event_t* e);
    static void dateUpCb(lv_event_t* e);
    static void dateDownCb(lv_event_t* e);
    static void dateSaveCb(lv_event_t* e);
    static void brightSliderCb(lv_event_t* e);
    static void brightSaveCb(lv_event_t* e);
    static void themeCb(lv_event_t* e);
    static void backCb(lv_event_t* e);
    static void cancelEditCb(lv_event_t* e);

    void updateTimeLabels();
    void updateDateLabels();
    void updateCounterLabel();
};

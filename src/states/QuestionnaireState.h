#pragma once
#include "states/IState.h"
#include <lvgl.h>
#include <ArduinoJson.h>

class QuestionItem;

class QuestionnaireState : public IState {
public:

    QuestionnaireState(App* app, JsonObjectConst questJson, bool cancellable = true)
        : IState(app), _questJson(questJson), _cancellable(cancellable) {}

    void enter() override;
    void update() override;
    void exit() override;
    StateType type() const override { return StateType::Questionnaire; }

private:
    JsonObjectConst _questJson;
    bool _cancellable;
    QuestionItem* _currentItem = nullptr;
    uint8_t _itemIdx = 0;
    uint8_t _itemCount = 0;
    lv_obj_t* _content = nullptr;

    void showItem();
    void onItemDone(int16_t response, unsigned long durationMs);
    void cleanupItem();

    static void itemCallback(int16_t response, unsigned long durationMs);
    static void cancelCb(lv_event_t* e);

    static QuestionnaireState* _activeInstance;
};

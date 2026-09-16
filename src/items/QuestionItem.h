#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <ArduinoJson.h>
#include "config.h"

typedef void (*ItemDoneCallback)(int16_t response, unsigned long durationMs);

class QuestionItem {
public:
    virtual ~QuestionItem();
    virtual void build(lv_obj_t* parent) = 0;
    virtual void destroy();
    virtual bool isComplete() const { return _valueSet; }
    virtual int16_t value() const { return _value; }
    virtual const char* typeName() const = 0;
    void setCallback(ItemDoneCallback cb) { _callback = cb; }
    const char* name()   const { return _name; }
    const char* prompt() const { return _prompt; }
    static QuestionItem* createFromJson(JsonObjectConst itemDef);

protected:
    char _name[MAX_NAME_LEN] = {};
    char _prompt[MAX_PROMPT_LEN] = {};
    int16_t _value = 0;
    bool _valueSet = false;
    bool _cancellable = true;
    unsigned long _startTime = 0;
    ItemDoneCallback _callback = nullptr;
    lv_obj_t* _continueBtn = nullptr;

    void startTimer() { _startTime = millis(); }
    void onContinue();
    void enableContinue();
    void addContinueButton(lv_obj_t* parent, bool disabled);

public:
    void setCancellable(bool c) { _cancellable = c; }

protected:
    static void continueCb(lv_event_t* e);
    static void selectionCb(lv_event_t* e);
};

class LikertItem : public QuestionItem {
public:
    void build(lv_obj_t* parent) override;
    const char* typeName() const override { return "likert"; }

    char leftLabel[MAX_LABEL_LEN] = {};
    char rightLabel[MAX_LABEL_LEN] = {};
    uint8_t points = 5;

private:
    void highlight(lv_obj_t* row);
};

class VASItem : public QuestionItem {
public:
    void build(lv_obj_t* parent) override;
    const char* typeName() const override { return "vas"; }

    char leftLabel[MAX_LABEL_LEN] = {};
    char rightLabel[MAX_LABEL_LEN] = {};

private:
    lv_obj_t* _slider = nullptr;
    static void sliderCb(lv_event_t* e);
};

class ChoiceItem : public QuestionItem {
public:
    void build(lv_obj_t* parent) override;
    const char* typeName() const override { return "options"; }

    char options[MAX_OPTIONS_PER_ITEM][MAX_OPTION_LEN] = {};
    uint8_t optionCount = 0;

protected:
    static void rowClickedCb(lv_event_t* e);
    virtual void onRowClicked(uint8_t idx, lv_obj_t* row);
    void setRowChecked(lv_obj_t* row, bool checked);
};

class MultiChoiceItem : public ChoiceItem {
public:
    const char* typeName() const override { return "multi_options"; }

protected:
    void onRowClicked(uint8_t idx, lv_obj_t* row) override;
};

class NumericItem : public QuestionItem {
public:
    void build(lv_obj_t* parent) override;
    const char* typeName() const override { return "numeric"; }

    int16_t minVal = 0;
    int16_t maxVal = 99;

private:
    lv_obj_t* _valueLbl = nullptr;
    void updateLabel();
    static void incCb(lv_event_t* e);
    static void decCb(lv_event_t* e);
};

class TextItem : public QuestionItem {
public:
    void build(lv_obj_t* parent) override;
    bool isComplete() const override { return true; }
    const char* typeName() const override { return "text"; }
};

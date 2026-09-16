// LockScreenState — PIN-Numpad Eingabe
#include "states/LockScreenState.h"
#include "App.h"
#include "ui/Styles.h"
#include "fonts_de.h"

void LockScreenState::enter() {
    _pinBuffer = "";
    _showPin = false;
    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    createCancelX(lv_screen_active(), cancelCb, this);

    createStyledLabel(lv_screen_active(), "PIN", COLOR_TEXT, &font_de_14, LV_ALIGN_TOP_MID, 0, 10);

    _displayLbl = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(_displayLbl, &font_de_20, 0);
    lv_obj_set_style_text_color(_displayLbl, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_text_letter_space(_displayLbl, 6, 0);
    lv_obj_align(_displayLbl, LV_ALIGN_TOP_MID, 10, 32);

    lv_obj_t* eyeBtn = lv_button_create(lv_screen_active());
    lv_obj_set_size(eyeBtn, 28, 28);
    lv_obj_set_style_bg_color(eyeBtn, lv_color_hex(COLOR_BTN_BG), 0);
    lv_obj_set_style_radius(eyeBtn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(eyeBtn, 0, 0);
    lv_obj_set_style_shadow_width(eyeBtn, 0, 0);
    lv_obj_align(eyeBtn, LV_ALIGN_TOP_MID, -60, 32);
    lv_obj_add_event_cb(eyeBtn, eyeCb, LV_EVENT_CLICKED, this);
    _eyeLbl = lv_label_create(eyeBtn);
    lv_label_set_text(_eyeLbl, LV_SYMBOL_EYE_OPEN);
    lv_obj_set_style_text_color(_eyeLbl, lv_color_hex(colorText()), 0);
    lv_obj_set_style_text_font(_eyeLbl, &font_de_14, 0);
    lv_obj_center(_eyeLbl);

    updateDisplay();

    lv_obj_t* pad = lv_obj_create(lv_screen_active());
    lv_obj_set_size(pad, 186, 152);
    lv_obj_align(pad, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_flex_flow(pad, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(pad, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(pad, 0, 0);
    lv_obj_set_style_pad_row(pad, 4, 0);
    lv_obj_set_style_pad_column(pad, 5, 0);
    lv_obj_set_style_bg_opa(pad, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(pad, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(pad, LV_SCROLLBAR_MODE_OFF);

    for (uint8_t d = 1; d <= 9; d++) {
        char buf[2]; snprintf(buf, sizeof(buf), "%d", d);
        lv_obj_t* btn = createNumpadButton(pad, buf);
        lv_obj_set_user_data(btn, (void*)(uintptr_t)d);
        lv_obj_add_event_cb(btn, digitCb, LV_EVENT_CLICKED, this);
    }

    lv_obj_t* spacer = lv_obj_create(pad);
    lv_obj_set_size(spacer, 54, 32);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, (lv_obj_flag_t)(LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));

    lv_obj_t* zeroBtn = createNumpadButton(pad, "0");
    lv_obj_set_user_data(zeroBtn, (void*)0);
    lv_obj_add_event_cb(zeroBtn, digitCb, LV_EVENT_CLICKED, this);

    lv_obj_t* clearBtn = createNumpadButton(pad, LV_SYMBOL_BACKSPACE);
    lv_obj_add_event_cb(clearBtn, clearCb, LV_EVENT_CLICKED, this);
}

void LockScreenState::update() {}

void LockScreenState::exit() {
    _displayLbl = nullptr;
    _eyeLbl = nullptr;
}

void LockScreenState::digitCb(lv_event_t* e) {
    LockScreenState* self = (LockScreenState*)lv_event_get_user_data(e);
    uint8_t digit = (uint8_t)(uintptr_t)lv_obj_get_user_data(
        (lv_obj_t*)lv_event_get_target(e));
    self->onDigit(digit);
}

void LockScreenState::clearCb(lv_event_t* e) {
    LockScreenState* self = (LockScreenState*)lv_event_get_user_data(e);
    if (self->_pinBuffer.length() > 0) {
        self->_pinBuffer.remove(self->_pinBuffer.length() - 1);
        self->updateDisplay();
    }
}

void LockScreenState::cancelCb(lv_event_t* e) {
    LockScreenState* self = (LockScreenState*)lv_event_get_user_data(e);

    if (self->_action == Action::UnlockDelete) {
        self->_app->requestState(StateType::Settings);
    } else {
        self->_app->requestState(StateType::Home);
    }
}

void LockScreenState::eyeCb(lv_event_t* e) {
    LockScreenState* self = (LockScreenState*)lv_event_get_user_data(e);
    self->_showPin = !self->_showPin;
    if (self->_eyeLbl) {
        lv_label_set_text(self->_eyeLbl,
                          self->_showPin ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
    }
    self->updateDisplay();
}

void LockScreenState::onDigit(uint8_t digit) {
    if (_pinBuffer.length() >= MAX_PIN_LENGTH) return;
    _pinBuffer += String(digit);
    updateDisplay();
    checkPin();
}

void LockScreenState::checkPin() {
    const char* correct = _app->config().pin();
    if (_pinBuffer.length() < strlen(correct)) return;

    if (_pinBuffer == correct) {

        switch (_action) {
            case Action::UnlockSettings:
                _app->requestState(StateType::Settings);
                break;
            case Action::UnlockDelete:
                _app->data().deleteAll();
                Serial.println("Daten geloescht (PIN korrekt)");

                _app->requestState(StateType::Settings);
                break;
        }
    } else {

        _pinBuffer = "";
        updateDisplay();
    }
}

void LockScreenState::updateDisplay() {
    if (!_displayLbl) return;
    String display;
    size_t shown = _pinBuffer.length();
    size_t total = 4;
    for (size_t i = 0; i < total; i++) {
        if (i > 0) display += " ";
        if (i < shown) {
            display += _showPin ? String(_pinBuffer[i]) : String("*");
        } else {
            display += "_";
        }
    }
    lv_label_set_text(_displayLbl, display.c_str());
}

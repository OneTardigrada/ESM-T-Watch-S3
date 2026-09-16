// SettingsState — Einstellungs-Menue mit Uhrzeit/Datum-Editor
#include "states/SettingsState.h"
#include "App.h"
#include "ui/Styles.h"
#include "fonts_de.h"
#include <LilyGoLib.h>

enum SettingsAction : uint8_t {
    ACT_TIME = 0, ACT_DATE = 1, ACT_BRIGHTNESS = 2, ACT_USBMODE = 3, ACT_DELETE = 4, ACT_BACK = 5
};

void SettingsState::enter() {
    _timeEditing = false;
    _dateEditing = false;
    showMainMenu();
}

void SettingsState::showMainMenu() {
    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    createStyledLabel(lv_screen_active(), "Einstellungen", COLOR_TEXT, &font_de_14, LV_ALIGN_TOP_LEFT, 16, 8);

    char buf[48];
    snprintf(buf, sizeof(buf), "ID: %s", _app->config().subjectId());
    lv_obj_t* info = createStyledLabel(lv_screen_active(), buf, colorTextDim(), &font_de_14, LV_ALIGN_TOP_LEFT, 16, 26);

    char brightLabel[40];
    snprintf(brightLabel, sizeof(brightLabel), LV_SYMBOL_IMAGE "  Helligkeit: %d%%",
             (int)((_app->brightness() * 100 + 127) / 255));
    char usbLabel[40];
    snprintf(usbLabel, sizeof(usbLabel), LV_SYMBOL_USB "  USB-Modus: %s",
             _app->isUsbMode() ? "AN" : "AUS");
    const char* labels[] = {
        LV_SYMBOL_REFRESH "  Uhrzeit",
        LV_SYMBOL_LIST "  Datum",
        brightLabel,
        usbLabel,
        LV_SYMBOL_TRASH "  Daten l\xC3\xB6schen"
    };

    lv_obj_t* col = lv_obj_create(lv_screen_active());
    lv_obj_set_size(col, 200, LV_SIZE_CONTENT);
    lv_obj_align(col, LV_ALIGN_TOP_MID, -4, 50);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(col, 6, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(col, LV_SCROLLBAR_MODE_OFF);

    for (uint8_t i = 0; i < 5; i++) {
        lv_obj_t* btn = lv_button_create(col);
        lv_obj_set_size(btn, 200, 36);
        lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_BTN_BG), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(btn, 14, 0);
        lv_obj_set_style_pad_left(btn, 14, 0);
        lv_obj_set_style_pad_right(btn, 8, 0);
        lv_obj_set_style_pad_ver(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_add_event_cb(btn, btnCb, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(btn, (void*)(uintptr_t)i);
        lv_obj_t* lbl = createStyledLabel(btn, labels[i], colorText(), &font_de_14, LV_ALIGN_LEFT_MID, 0, 0);
    }

    lv_obj_t* xBtn = createCancelX(lv_screen_active(), backCb, this);
    lv_obj_align(xBtn, LV_ALIGN_TOP_RIGHT, -24, 8);
}

void SettingsState::update() {}

void SettingsState::exit() {
    _hourLbl = _minLbl = _dayLbl = _monLbl = _yearLbl = _counterLbl = nullptr;
    _brightLbl = _brightSlider = nullptr;
}

void SettingsState::btnCb(lv_event_t* e) {
    SettingsState* self = (SettingsState*)lv_event_get_user_data(e);
    uint8_t action = (uint8_t)(uintptr_t)lv_obj_get_user_data(
        (lv_obj_t*)lv_event_get_target(e));
    self->onButton(action);
}

void SettingsState::onButton(uint8_t action) {
    switch (action) {
        case ACT_TIME: showTimeEditor(); break;
        case ACT_DATE: showDateEditor(); break;
        case ACT_BRIGHTNESS: showBrightnessEditor(); break;
        case ACT_USBMODE:

            _app->setUsbMode(!_app->isUsbMode());
            showMainMenu();
            break;
        case ACT_DELETE:
            _app->requestState(StateType::LockScreen, "delete");
            break;
    }
}

void SettingsState::updateCounterLabel() {
    if (!_counterLbl) return;
    if (!_app->config().isLoaded()) {
        lv_label_set_text(_counterLbl, "Keine Config");
        return;
    }

    uint16_t esmCount = 0;
    uint16_t cravCount = 0;
    uint8_t qc = _app->config().questionnaireCount();
    for (uint8_t i = 0; i < qc; i++) {
        JsonObjectConst q = _app->config().getQuestionnaire(i);
        if (q.isNull()) continue;
        const char* name = q["name"] | "";
        JsonArrayConst items = q["items"];
        if (items.isNull() || items.size() == 0) continue;

        const char* lastItem = items[items.size() - 1]["name"] | "";
        uint16_t c = _app->data().countCompleted(name, lastItem);
        if (q["event"] | false)
            cravCount += c;
        else
            esmCount += c;
    }

    char buf[48];
    snprintf(buf, sizeof(buf), "ESM: %d  Cravings: %d", esmCount, cravCount);
    lv_label_set_text(_counterLbl, buf);
}

void SettingsState::themeCb(lv_event_t* e) {
    SettingsState* self = (SettingsState*)lv_event_get_user_data(e);
    self->_app->toggleTheme();
}

void SettingsState::backCb(lv_event_t* e) {
    SettingsState* self = (SettingsState*)lv_event_get_user_data(e);
    self->_app->requestState(StateType::Home);
}

void SettingsState::cancelEditCb(lv_event_t* e) {
    SettingsState* self = (SettingsState*)lv_event_get_user_data(e);
    self->_timeEditing = false;
    self->_dateEditing = false;

    if (self->_brightSlider) {
        instance.setBrightness(self->_app->brightness());
        self->_brightSlider = nullptr;
        self->_brightLbl = nullptr;
    }
    self->showMainMenu();
}

void SettingsState::showTimeEditor() {
    _timeEditing = true;
    struct tm t;
    instance.rtc.getDateTime(&t);
    _editH = t.tm_hour; _editM = t.tm_min;

    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    createCancelX(lv_screen_active(), cancelEditCb, this);

    createStyledLabel(lv_screen_active(), "Uhrzeit", COLOR_TEXT, &font_de_14, LV_ALIGN_TOP_MID, 0, 19);

    lv_obj_t* hUp = lv_button_create(lv_screen_active());
    lv_obj_set_size(hUp, 56, 30); lv_obj_add_style(hUp, &styleBtn, 0);
    lv_obj_align(hUp, LV_ALIGN_CENTER, -38, -34);
    lv_obj_set_user_data(hUp, (void*)0);
    lv_obj_add_event_cb(hUp, timeUpCb, LV_EVENT_CLICKED, this);
    lv_obj_t* hul = lv_label_create(hUp); lv_label_set_text(hul, LV_SYMBOL_UP); lv_obj_center(hul);

    _hourLbl = createStyledLabel(lv_screen_active(), "", COLOR_TEXT, &font_de_20, LV_ALIGN_CENTER, -38, 5);

    lv_obj_t* hDown = lv_button_create(lv_screen_active());
    lv_obj_set_size(hDown, 56, 30); lv_obj_add_style(hDown, &styleBtn, 0);
    lv_obj_align(hDown, LV_ALIGN_CENTER, -38, 44);
    lv_obj_set_user_data(hDown, (void*)0);
    lv_obj_add_event_cb(hDown, timeDownCb, LV_EVENT_CLICKED, this);
    lv_obj_t* hdl = lv_label_create(hDown); lv_label_set_text(hdl, LV_SYMBOL_DOWN); lv_obj_center(hdl);

    createStyledLabel(lv_screen_active(), ":", COLOR_TEXT, &font_de_20, LV_ALIGN_CENTER, 0, 5);

    lv_obj_t* mUp = lv_button_create(lv_screen_active());
    lv_obj_set_size(mUp, 56, 30); lv_obj_add_style(mUp, &styleBtn, 0);
    lv_obj_align(mUp, LV_ALIGN_CENTER, 38, -34);
    lv_obj_set_user_data(mUp, (void*)1);
    lv_obj_add_event_cb(mUp, timeUpCb, LV_EVENT_CLICKED, this);
    lv_obj_t* mul = lv_label_create(mUp); lv_label_set_text(mul, LV_SYMBOL_UP); lv_obj_center(mul);

    _minLbl = createStyledLabel(lv_screen_active(), "", COLOR_TEXT, &font_de_20, LV_ALIGN_CENTER, 38, 5);

    lv_obj_t* mDown = lv_button_create(lv_screen_active());
    lv_obj_set_size(mDown, 56, 30); lv_obj_add_style(mDown, &styleBtn, 0);
    lv_obj_align(mDown, LV_ALIGN_CENTER, 38, 44);
    lv_obj_set_user_data(mDown, (void*)1);
    lv_obj_add_event_cb(mDown, timeDownCb, LV_EVENT_CLICKED, this);
    lv_obj_t* mdl = lv_label_create(mDown); lv_label_set_text(mdl, LV_SYMBOL_DOWN); lv_obj_center(mdl);

    lv_obj_t* saveBtn = lv_button_create(lv_screen_active());
    lv_obj_set_size(saveBtn, 140, 36); lv_obj_add_style(saveBtn, &styleBtn, 0);
    lv_obj_align(saveBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(saveBtn, timeSaveCb, LV_EVENT_CLICKED, this);
    lv_obj_t* sl = lv_label_create(saveBtn); lv_label_set_text(sl, LV_SYMBOL_OK " Setzen"); lv_obj_center(sl);

    updateTimeLabels();
}

void SettingsState::updateTimeLabels() {
    char buf[4];
    if (_hourLbl) { snprintf(buf, sizeof(buf), "%02d", _editH); lv_label_set_text(_hourLbl, buf); }
    if (_minLbl)  { snprintf(buf, sizeof(buf), "%02d", _editM); lv_label_set_text(_minLbl, buf);  }
}

void SettingsState::timeUpCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    uint8_t field = (uint8_t)(uintptr_t)lv_obj_get_user_data((lv_obj_t*)lv_event_get_target(e));
    if (field == 0) { s->_editH = (s->_editH + 1) % 24; }
    else            { s->_editM = (s->_editM + 1) % 60; }
    s->updateTimeLabels();
}

void SettingsState::timeDownCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    uint8_t field = (uint8_t)(uintptr_t)lv_obj_get_user_data((lv_obj_t*)lv_event_get_target(e));
    if (field == 0) { s->_editH = (s->_editH == 0) ? 23 : s->_editH - 1; }
    else            { s->_editM = (s->_editM == 0) ? 59 : s->_editM - 1; }
    s->updateTimeLabels();
}

void SettingsState::timeSaveCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    struct tm t;
    instance.rtc.getDateTime(&t);
    instance.rtc.setDateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                             s->_editH, s->_editM, 0);
    Serial.printf("Uhrzeit gesetzt: %02d:%02d\n", s->_editH, s->_editM);
    s->_timeEditing = false;
    s->showMainMenu();
}

void SettingsState::showDateEditor() {
    _dateEditing = true;
    struct tm t;
    instance.rtc.getDateTime(&t);
    _editD = t.tm_mday; _editMo = t.tm_mon + 1; _editY = t.tm_year + 1900;

    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    createCancelX(lv_screen_active(), cancelEditCb, this);

    createStyledLabel(lv_screen_active(), "Datum", COLOR_TEXT, &font_de_14, LV_ALIGN_TOP_MID, 0, 19);

    int xOff[] = {-72, 0, 72};
    lv_obj_t** lbls[] = {&_dayLbl, &_monLbl, &_yearLbl};
    const char* fieldLabels[] = {"Tag", "Mon", "Jahr"};

    for (int f = 0; f < 3; f++) {
        lv_obj_t* up = lv_button_create(lv_screen_active());
        lv_obj_set_size(up, 50, 26); lv_obj_add_style(up, &styleBtn, 0);
        lv_obj_align(up, LV_ALIGN_CENTER, xOff[f], -32);
        lv_obj_set_user_data(up, (void*)(uintptr_t)f);
        lv_obj_add_event_cb(up, dateUpCb, LV_EVENT_CLICKED, this);
        lv_obj_t* ul = lv_label_create(up); lv_label_set_text(ul, LV_SYMBOL_UP); lv_obj_center(ul);

        createStyledLabel(lv_screen_active(), fieldLabels[f], COLOR_TEXT_DIM, &font_de_14, LV_ALIGN_CENTER, xOff[f], -12);

        *lbls[f] = createStyledLabel(lv_screen_active(), "", COLOR_TEXT, &font_de_20, LV_ALIGN_CENTER, xOff[f], 8);

        lv_obj_t* dn = lv_button_create(lv_screen_active());
        lv_obj_set_size(dn, 50, 26); lv_obj_add_style(dn, &styleBtn, 0);
        lv_obj_align(dn, LV_ALIGN_CENTER, xOff[f], 32);
        lv_obj_set_user_data(dn, (void*)(uintptr_t)f);
        lv_obj_add_event_cb(dn, dateDownCb, LV_EVENT_CLICKED, this);
        lv_obj_t* dl = lv_label_create(dn); lv_label_set_text(dl, LV_SYMBOL_DOWN); lv_obj_center(dl);
    }

    lv_obj_t* saveBtn = lv_button_create(lv_screen_active());
    lv_obj_set_size(saveBtn, 140, 36); lv_obj_add_style(saveBtn, &styleBtn, 0);
    lv_obj_align(saveBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(saveBtn, dateSaveCb, LV_EVENT_CLICKED, this);
    lv_obj_t* sl = lv_label_create(saveBtn); lv_label_set_text(sl, LV_SYMBOL_OK " Setzen"); lv_obj_center(sl);

    updateDateLabels();
}

void SettingsState::updateDateLabels() {
    char buf[6];
    if (_dayLbl)  { snprintf(buf, sizeof(buf), "%02d", _editD);   lv_label_set_text(_dayLbl, buf);  }
    if (_monLbl)  { snprintf(buf, sizeof(buf), "%02d", _editMo);  lv_label_set_text(_monLbl, buf);  }
    if (_yearLbl) { snprintf(buf, sizeof(buf), "%04d", _editY);   lv_label_set_text(_yearLbl, buf); }
}

void SettingsState::dateUpCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    uint8_t f = (uint8_t)(uintptr_t)lv_obj_get_user_data((lv_obj_t*)lv_event_get_target(e));
    if (f == 0) { s->_editD = (s->_editD % 31) + 1; }
    else if (f == 1) { s->_editMo = (s->_editMo % 12) + 1; }
    else { s->_editY++; }
    s->updateDateLabels();
}

void SettingsState::dateDownCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    uint8_t f = (uint8_t)(uintptr_t)lv_obj_get_user_data((lv_obj_t*)lv_event_get_target(e));
    if (f == 0) { s->_editD = (s->_editD <= 1) ? 31 : s->_editD - 1; }
    else if (f == 1) { s->_editMo = (s->_editMo <= 1) ? 12 : s->_editMo - 1; }
    else { if (s->_editY > 2020) s->_editY--; }
    s->updateDateLabels();
}

void SettingsState::dateSaveCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    struct tm t;
    instance.rtc.getDateTime(&t);
    instance.rtc.setDateTime(s->_editY, s->_editMo, s->_editD,
                             t.tm_hour, t.tm_min, t.tm_sec);
    Serial.printf("Datum gesetzt: %02d.%02d.%04d\n", s->_editD, s->_editMo, s->_editY);
    s->_dateEditing = false;
    s->showMainMenu();
}

void SettingsState::showBrightnessEditor() {
    _editBright = _app->brightness();

    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    createCancelX(lv_screen_active(), cancelEditCb, this);

    createStyledLabel(lv_screen_active(), "Helligkeit", COLOR_TEXT, &font_de_14, LV_ALIGN_TOP_MID, 0, 19);

    _brightLbl = createStyledLabel(lv_screen_active(), "", COLOR_TEXT, &font_de_20, LV_ALIGN_CENTER, 0, -24);

    _brightSlider = lv_slider_create(lv_screen_active());
    lv_obj_set_width(_brightSlider, 200);
    lv_slider_set_range(_brightSlider, 8, 255);
    lv_slider_set_value(_brightSlider, _editBright, LV_ANIM_OFF);
    lv_obj_align(_brightSlider, LV_ALIGN_CENTER, 0, 16);
    applySliderStyle(_brightSlider);
    lv_obj_add_event_cb(_brightSlider, brightSliderCb, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t* saveBtn = lv_button_create(lv_screen_active());
    lv_obj_set_size(saveBtn, 140, 36); lv_obj_add_style(saveBtn, &styleBtn, 0);
    lv_obj_align(saveBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(saveBtn, brightSaveCb, LV_EVENT_CLICKED, this);
    lv_obj_t* sl = lv_label_create(saveBtn); lv_label_set_text(sl, LV_SYMBOL_OK " Setzen"); lv_obj_center(sl);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", (int)((_editBright * 100 + 127) / 255));
    lv_label_set_text(_brightLbl, buf);
}

void SettingsState::brightSliderCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    if (!s->_brightSlider) return;
    int v = lv_slider_get_value(s->_brightSlider);
    s->_editBright = (uint8_t)v;

    instance.setBrightness(s->_editBright);
    if (s->_brightLbl) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", (int)((s->_editBright * 100 + 127) / 255));
        lv_label_set_text(s->_brightLbl, buf);
    }
}

void SettingsState::brightSaveCb(lv_event_t* e) {
    SettingsState* s = (SettingsState*)lv_event_get_user_data(e);
    s->_app->setBrightness(s->_editBright);
    Serial.printf("Helligkeit gesetzt: %d\n", (int)s->_editBright);
    s->_brightSlider = nullptr;
    s->_brightLbl = nullptr;
    s->showMainMenu();
}

// HomeState — Hauptbildschirm mit Uhrzeit, Batterie, Settings, Craving
#include "states/HomeState.h"
#include "App.h"
#include "ui/Styles.h"
#include "fonts_de.h"
#include <LilyGoLib.h>

static const char* dayNamesDE[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

void HomeState::enter() {
    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    drawWallpaper(lv_screen_active());

    _battIcon = createStyledLabel(lv_screen_active(), LV_SYMBOL_BATTERY_FULL, COLOR_ACCENT, &font_de_14, LV_ALIGN_TOP_LEFT, 10, 10);
    _battLbl  = createStyledLabel(lv_screen_active(), "0%", COLOR_TEXT, &font_de_14, LV_ALIGN_TOP_LEFT, 38, 11);

    _usbIcon = createStyledLabel(lv_screen_active(), LV_SYMBOL_USB, COLOR_ACCENT, &font_de_14, LV_ALIGN_TOP_MID, 0, 11);
    lv_obj_add_flag(_usbIcon, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* settingsBtn = lv_button_create(lv_screen_active());
    lv_obj_set_size(settingsBtn, 36, 36);
    lv_obj_align(settingsBtn, LV_ALIGN_TOP_RIGHT, -6, 4);
    lv_obj_set_style_bg_opa(settingsBtn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(settingsBtn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(settingsBtn, LV_OPA_TRANSP, 0);
    lv_obj_add_event_cb(settingsBtn, settingsCb, LV_EVENT_CLICKED, _app);
    createStyledLabel(settingsBtn, LV_SYMBOL_SETTINGS, COLOR_TEXT_DIM, &font_de_14, LV_ALIGN_CENTER, 0, 0);

    _timeLbl = createStyledLabel(lv_screen_active(), "00:00", COLOR_TEXT, &font_de_48, LV_ALIGN_CENTER, 0, -12);

    _dateLbl = createStyledLabel(lv_screen_active(), "", colorTextDim(), &font_de_14, LV_ALIGN_CENTER, 0, 22);

    if (_app->config().eventQuestionnaireCount() > 0) {
        lv_obj_t* cravBtn = createAccentButton(lv_screen_active(), 52, 52, LV_RADIUS_CIRCLE,
                                               LV_SYMBOL_EDIT, &font_de_20, cravingCb, _app);
        lv_obj_align(cravBtn, LV_ALIGN_BOTTOM_RIGHT, -12, -12);
    }

    if (!_app->config().isLoaded()) {
        lv_obj_t* hint = createStyledLabel(lv_screen_active(), "Keine Config.\nBitte per USB senden.",
                                           COLOR_WARNING, &font_de_14, LV_ALIGN_BOTTOM_MID, 0, -16);
        lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(hint, 200);
        lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    }

    _alarmIcon = createAccentButton(lv_screen_active(), 52, 52, LV_RADIUS_CIRCLE,
                                    LV_SYMBOL_LIST, &font_de_20, alarmIconCb, _app);
    lv_obj_align(_alarmIcon, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_add_flag(_alarmIcon, LV_OBJ_FLAG_HIDDEN);

    update();
}

void HomeState::update() {
    struct tm t;
    instance.rtc.getDateTime(&t);
    int  batt = (int)_app->batteryPercent();
    bool chg  = _app->isCharging();

    if (_timeLbl && (t.tm_hour != _lastHour || t.tm_min != _lastMin)) {
        _lastHour = t.tm_hour; _lastMin = t.tm_min;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);
        lv_label_set_text(_timeLbl, buf);
    }

    if (_dateLbl && (t.tm_wday != _lastWday || t.tm_mday != _lastMday || t.tm_mon != _lastMon)) {
        _lastWday = t.tm_wday; _lastMday = t.tm_mday; _lastMon = t.tm_mon;
        char buf[16];
        const char* wd = (t.tm_wday < 7) ? dayNamesDE[t.tm_wday] : "??";
        snprintf(buf, sizeof(buf), "%s, %02d.%02d.", wd, t.tm_mday, t.tm_mon + 1);
        lv_label_set_text(_dateLbl, buf);
    }

    if (batt != _lastBatt || (int8_t)chg != _lastChg) {
        _lastBatt = batt; _lastChg = (int8_t)chg;
        updateBattIcon(batt, chg);
    }
    updateAlarmBanner();

    if (_usbIcon) {
        int8_t um = _app->isUsbMode() ? 1 : 0;
        if (um != _lastUsbMode) {
            _lastUsbMode = um;
            if (um) lv_obj_remove_flag(_usbIcon, LV_OBJ_FLAG_HIDDEN);
            else    lv_obj_add_flag(_usbIcon, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void HomeState::exit() {
    _timeLbl = _dateLbl = _battLbl = _battIcon = _alarmIcon = _usbIcon = nullptr;

    _lastHour = _lastMin = _lastWday = _lastMday = _lastMon = -1;
    _lastBatt = -1; _lastChg = -1; _lastUsbMode = -1; _lastAlarmVisible = -1;
}

void HomeState::updateBattIcon(int bp, bool chg) {
    if (!_battIcon || !_battLbl) return;
    if (bp < 0) bp = 0; if (bp > 100) bp = 100;
    char buf[6]; snprintf(buf, sizeof(buf), "%d%%", bp);
    lv_label_set_text(_battLbl, buf);

    if (chg) {
        lv_label_set_text(_battIcon, LV_SYMBOL_CHARGE);
        lv_obj_set_style_text_color(_battIcon, lv_color_hex(COLOR_ACCENT), 0);
    } else if (bp > 75) {
        lv_label_set_text(_battIcon, LV_SYMBOL_BATTERY_FULL);
        lv_obj_set_style_text_color(_battIcon, lv_color_hex(COLOR_ACCENT), 0);
    } else if (bp > 50) {
        lv_label_set_text(_battIcon, LV_SYMBOL_BATTERY_3);
        lv_obj_set_style_text_color(_battIcon, lv_color_hex(COLOR_ACCENT), 0);
    } else if (bp > 25) {
        lv_label_set_text(_battIcon, LV_SYMBOL_BATTERY_2);
        lv_obj_set_style_text_color(_battIcon, lv_color_hex(COLOR_WARNING), 0);
    } else if (bp > 10) {
        lv_label_set_text(_battIcon, LV_SYMBOL_BATTERY_1);
        lv_obj_set_style_text_color(_battIcon, lv_color_hex(COLOR_ERROR), 0);
    } else {
        lv_label_set_text(_battIcon, LV_SYMBOL_BATTERY_EMPTY);
        lv_obj_set_style_text_color(_battIcon, lv_color_hex(0xFF0000), 0);
    }
}

void HomeState::settingsCb(lv_event_t* e) {
    App* app = (App*)lv_event_get_user_data(e);
    app->requestState(StateType::LockScreen, "settings");
}

void HomeState::cravingCb(lv_event_t* e) {
    App* app = (App*)lv_event_get_user_data(e);
    app->startEventQuestionnaire(0);
}

void HomeState::updateAlarmBanner() {
    if (!_alarmIcon) return;
    int8_t pending = _app->schedule().pendingAlarmIndex();
    int8_t visible = (pending >= 0) ? 1 : 0;
    if (visible == _lastAlarmVisible) return;
    _lastAlarmVisible = visible;
    if (visible) lv_obj_remove_flag(_alarmIcon, LV_OBJ_FLAG_HIDDEN);
    else         lv_obj_add_flag(_alarmIcon, LV_OBJ_FLAG_HIDDEN);
}

void HomeState::alarmIconCb(lv_event_t* e) {
    App* app = (App*)lv_event_get_user_data(e);
    int8_t pending = app->schedule().pendingAlarmIndex();
    if (pending < 0) return;
    const char* questName = app->schedule().alarmQuestName(pending);
    app->showNotification(questName, pending);
}

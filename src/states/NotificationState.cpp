// NotificationState — ESM-Alarm-Benachrichtigung
#include "states/NotificationState.h"
#include "App.h"
#include "ui/Styles.h"
#include "fonts_de.h"

void NotificationState::enter() {
    _app->power().resetInactivityTimer();
    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    createCancelX(lv_screen_active(), declineCb, this);

    lv_obj_t* title = lv_label_create(lv_screen_active());
    lv_label_set_text(title, _questName);
    lv_obj_add_style(title, &styleTitle, 0);
    lv_obj_set_width(title, DISPLAY_WIDTH - 56);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 58);

    lv_obj_t* msg = lv_label_create(lv_screen_active());
    lv_label_set_text(msg, "Neuer Fragebogen\nverf\xC3\xBCgbar");
    lv_obj_add_style(msg, &stylePrompt, 0);
    lv_obj_set_width(msg, 220);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* acceptBtn = createAccentButton(lv_screen_active(), 180, 44, 12,
                                              LV_SYMBOL_RIGHT " Fragebogen starten", &font_de_14,
                                              acceptCb, this);
    lv_obj_align(acceptBtn, LV_ALIGN_BOTTOM_MID, 0, -16);
}

void NotificationState::update() {

    if (millis() - _app->power().lastActivityMs() >= NOTIF_DISPLAY_TIMEOUT_MS) {
        _app->requestState(StateType::Home);
    }
}

void NotificationState::exit() {}

void NotificationState::acceptCb(lv_event_t* e) {
    NotificationState* self = (NotificationState*)lv_event_get_user_data(e);

    self->_app->schedule().markAnswered(self->_alarmIdx);
    self->_app->startScheduledQuestionnaire(self->_questName);
}

void NotificationState::declineCb(lv_event_t* e) {
    NotificationState* self = (NotificationState*)lv_event_get_user_data(e);

    const char* questName = self->_app->schedule().alarmQuestName(self->_alarmIdx);
    bool log = self->_app->schedule().markHandled(self->_alarmIdx);
    if (log) {
        self->_app->logNotAnswered(questName);
    }
    self->_app->requestState(StateType::Home);
}

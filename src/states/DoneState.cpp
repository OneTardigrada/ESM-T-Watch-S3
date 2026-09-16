// DoneState — Abschluss-Screen mit Haekchen
#include "states/DoneState.h"
#include "App.h"
#include "ui/Styles.h"
#include "fonts_de.h"

void DoneState::enter() {
    resetScreen(lv_screen_active());
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    lv_obj_t* icon = createStyledLabel(lv_screen_active(), LV_SYMBOL_OK, COLOR_ACCENT, &font_de_48, LV_ALIGN_CENTER, 0, -30);
    (void)icon;

    lv_obj_t* msg = lv_label_create(lv_screen_active());
    lv_label_set_text(msg, _message);
    lv_obj_add_style(msg, &stylePrompt, 0);
    lv_obj_set_width(msg, 220);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 20);

    lv_obj_t* btn = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn, 110, 42);
    lv_obj_add_style(btn, &styleBtn, 0);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn, okCb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "OK");
    lv_obj_center(lbl);
}

void DoneState::update() {}

void DoneState::exit() {}

void DoneState::okCb(lv_event_t* e) {
    DoneState* self = (DoneState*)lv_event_get_user_data(e);
    self->_app->requestState(StateType::Home);
}

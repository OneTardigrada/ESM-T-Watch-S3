// QuestionnaireState — Fragebogen-Engine mit polymorphen Items
#include "states/QuestionnaireState.h"
#include "App.h"
#include "items/QuestionItem.h"
#include "ui/Styles.h"
#include "fonts_de.h"
#include <LilyGoLib.h>
#include <cstring>

QuestionnaireState* QuestionnaireState::_activeInstance = nullptr;

void QuestionnaireState::enter() {
    _activeInstance = this;
    _itemIdx = 0;

    JsonArrayConst items = _questJson["items"];
    _itemCount = items.isNull() ? 0 : (uint8_t)items.size();

    if (_itemCount == 0) {
        _app->showDone("Keine Items konfiguriert.");
        return;
    }

    resetScreen(lv_screen_active());
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_text_font(lv_screen_active(), &font_de_14, 0);

    _content = lv_obj_create(lv_screen_active());
    lv_obj_set_size(_content, 240, 194);
    lv_obj_align(_content, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(_content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(_content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(_content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(_content, 4, 0);
    lv_obj_set_style_pad_bottom(_content, 0, 0);
    lv_obj_set_style_pad_row(_content, 4, 0);
    lv_obj_set_scrollbar_mode(_content, LV_SCROLLBAR_MODE_OFF);

    if (_cancellable) {
        lv_obj_t* xBtn = createCancelX(lv_screen_active(), cancelCb, this);
        lv_obj_align(xBtn, LV_ALIGN_BOTTOM_LEFT, 8, -8);
    }

    showItem();
}

void QuestionnaireState::showItem() {
    cleanupItem();

    lv_obj_clean(_content);

    lv_obj_t* progBar = lv_bar_create(_content);
    lv_obj_set_width(progBar, 224);
    lv_obj_set_height(progBar, 4);
    lv_bar_set_range(progBar, 0, _itemCount);
    lv_bar_set_value(progBar, _itemIdx + 1, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progBar, lv_color_hex(COLOR_BTN_BG), 0);
    lv_obj_set_style_bg_color(progBar, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progBar, 2, 0);
    lv_obj_set_style_radius(progBar, 2, LV_PART_INDICATOR);

    JsonArrayConst items = _questJson["items"];
    JsonObjectConst itemDef = items[_itemIdx];
    _currentItem = QuestionItem::createFromJson(itemDef);

    if (!_currentItem) {
        _app->showDone("Item-Fehler!");
        return;
    }

    lv_obj_t* promptLbl = lv_label_create(_content);
    lv_label_set_text(promptLbl, _currentItem->prompt());
    lv_obj_set_style_text_color(promptLbl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_text_font(promptLbl, &font_de_14, 0);
    lv_label_set_long_mode(promptLbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(promptLbl, 224);
    lv_obj_set_style_text_align(promptLbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_bottom(promptLbl, 2, 0);

    _currentItem->setCancellable(_cancellable);
    _currentItem->setCallback(itemCallback);
    _currentItem->build(_content);

    _app->power().resetInactivityTimer();
}

void QuestionnaireState::itemCallback(int16_t response, unsigned long durationMs) {
    if (_activeInstance) {
        _activeInstance->onItemDone(response, durationMs);
    }
}

void QuestionnaireState::onItemDone(int16_t response, unsigned long durationMs) {

    char ts[20];
    _app->formatNow(ts, sizeof(ts));

    const char* questName = _questJson["name"] | "unknown";
    const char* typeName  = _currentItem->typeName();

    if (strcmp(typeName, "multi_options") == 0) {
        uint16_t mask = (uint16_t)response;
        if (mask == 0) {

            _app->data().saveResponse(ts, _app->config().subjectId(), questName,
                _currentItem->name(), typeName, _currentItem->name(),
                0, durationMs);
        } else {
            for (uint8_t i = 0; i < 16; i++) {
                if (mask & (1U << i)) {
                    _app->data().saveResponse(ts, _app->config().subjectId(), questName,
                        _currentItem->name(), typeName, _currentItem->name(),
                        (int16_t)(i + 1), durationMs);
                }
            }
        }
        Serial.printf("%s %s mask=0x%04X\n", questName, _currentItem->name(), mask);
    } else {
        _app->data().saveResponse(
            ts,
            _app->config().subjectId(),
            questName,
            _currentItem->name(),
            typeName,
            _currentItem->name(),
            response,
            durationMs
        );
        Serial.printf("%s %s = %d\n", questName, _currentItem->name(), response);
    }

    _itemIdx++;
    if (_itemIdx < _itemCount) {
        showItem();
    } else {

        const char* configMsg = _questJson["done_message"] | nullptr;
        Serial.printf("[DoneMsg] quest=%s done_message=%s\n",
                      questName,
                      (configMsg ? configMsg : "<missing>"));
        char msg[96];
        if (configMsg && configMsg[0]) {
            strncpy(msg, configMsg, sizeof(msg) - 1);
            msg[sizeof(msg) - 1] = '\0';
        } else {
            snprintf(msg, sizeof(msg), "Danke!\n%s abgeschlossen.",
                     _questJson["display_name"] | questName);
        }
        _app->showDone(msg);
    }
}

void QuestionnaireState::update() {

}

void QuestionnaireState::cleanupItem() {
    if (_currentItem) {
        _currentItem->destroy();
        delete _currentItem;
        _currentItem = nullptr;
    }
}

void QuestionnaireState::exit() {
    cleanupItem();
    _content = nullptr;
    _activeInstance = nullptr;
}

void QuestionnaireState::cancelCb(lv_event_t* e) {
    QuestionnaireState* self = (QuestionnaireState*)lv_event_get_user_data(e);
    self->_app->requestState(StateType::Home);
}

// QuestionItem — Implementierung aller Fragetypen
#include "items/QuestionItem.h"
#include "ui/Styles.h"
#include "fonts_de.h"
#include <cstring>

QuestionItem::~QuestionItem() {}

void QuestionItem::destroy() {
    _continueBtn = nullptr;
}

void QuestionItem::onContinue() {
    if (!_valueSet || !_callback) return;
    unsigned long duration = millis() - _startTime;
    _callback(_value, duration);
}

void QuestionItem::enableContinue() {
    if (_continueBtn && _valueSet) {
        lv_obj_remove_state(_continueBtn, LV_STATE_DISABLED);
    }
}

void QuestionItem::addContinueButton(lv_obj_t* parent, bool disabled) {
    lv_obj_t* scr = lv_screen_active();
    _continueBtn = lv_button_create(scr);

    int btnW = 120;
    int btnOffX = _cancellable ? 19 : 0;
    lv_obj_set_size(_continueBtn, btnW, 38);
    lv_obj_align(_continueBtn, LV_ALIGN_BOTTOM_MID, btnOffX, -8);

    lv_obj_set_style_bg_color(_continueBtn, lv_color_hex(COLOR_ACCENT), 0);
    lv_obj_set_style_bg_opa(_continueBtn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(_continueBtn, 12, 0);
    lv_obj_set_style_shadow_width(_continueBtn, 0, 0);
    lv_obj_add_event_cb(_continueBtn, continueCb, LV_EVENT_CLICKED, this);

    lv_obj_t* lbl = lv_label_create(_continueBtn);
    lv_label_set_text(lbl, LV_SYMBOL_RIGHT " Weiter");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl, &font_de_14, 0);
    lv_obj_center(lbl);

    if (disabled) {
        lv_obj_add_state(_continueBtn, LV_STATE_DISABLED);
        lv_obj_add_style(_continueBtn, &styleBtnDisabled, LV_STATE_DISABLED);
    }
}

void QuestionItem::continueCb(lv_event_t* e) {
    QuestionItem* item = (QuestionItem*)lv_event_get_user_data(e);
    if (item) item->onContinue();
}

void QuestionItem::selectionCb(lv_event_t* e) {
    QuestionItem* item = (QuestionItem*)lv_event_get_user_data(e);
    if (!item) return;
    lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);
    int16_t val = (int16_t)(intptr_t)lv_obj_get_user_data(target);
    item->_value = val;
    item->_valueSet = true;
    item->enableContinue();

    lv_obj_t* parent = lv_obj_get_parent(target);
    uint32_t cnt = lv_obj_get_child_count(parent);
    for (uint32_t i = 0; i < cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        if (!lv_obj_check_type(child, &lv_button_class)) continue;
        if (lv_obj_get_user_data(child) == nullptr) continue;
        bool sel = (child == target);
        lv_color_t bgC = lv_color_hex(sel ? COLOR_ACCENT : 0xFFFFFF);
        lv_obj_set_style_bg_color(child, bgC, 0);
        lv_obj_set_style_bg_color(child, bgC, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(child, bgC, LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(child, bgC, LV_STATE_FOCUSED | LV_STATE_PRESSED);
        lv_obj_set_style_border_color(child, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_border_color(child, lv_color_hex(COLOR_ACCENT), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(child, lv_color_hex(COLOR_ACCENT), LV_STATE_FOCUSED);
        lv_obj_t* lbl = lv_obj_get_child(child, 0);
        if (lbl) lv_obj_set_style_text_color(lbl, lv_color_hex(sel ? 0x000000 : COLOR_TEXT), 0);
    }
}

QuestionItem* QuestionItem::createFromJson(JsonObjectConst def) {
    const char* type = def["type"] | "text";
    const char* name = def["name"] | "unnamed";
    const char* prompt = def["prompt"] | "";

    if (strcmp(type, "likert") == 0) {
        LikertItem* item = new LikertItem();
        strncpy(item->_name, name, MAX_NAME_LEN - 1);
        strncpy(item->_prompt, prompt, MAX_PROMPT_LEN - 1);
        strncpy(item->leftLabel, def["min_label"] | (def["left_label"] | ""), MAX_LABEL_LEN - 1);
        strncpy(item->rightLabel, def["max_label"] | (def["right_label"] | ""), MAX_LABEL_LEN - 1);
        item->points = def["scale_max"] | (def["points"] | 5);
        return item;
    }
    if (strcmp(type, "vas") == 0) {
        VASItem* item = new VASItem();
        strncpy(item->_name, name, MAX_NAME_LEN - 1);
        strncpy(item->_prompt, prompt, MAX_PROMPT_LEN - 1);
        strncpy(item->leftLabel, def["left_label"] | "", MAX_LABEL_LEN - 1);
        strncpy(item->rightLabel, def["right_label"] | "", MAX_LABEL_LEN - 1);
        return item;
    }
    if (strcmp(type, "options") == 0 || strcmp(type, "multi_options") == 0) {
        ChoiceItem* item = (strcmp(type, "multi_options") == 0)
            ? new MultiChoiceItem() : new ChoiceItem();
        strncpy(item->_name, name, MAX_NAME_LEN - 1);
        strncpy(item->_prompt, prompt, MAX_PROMPT_LEN - 1);
        JsonArrayConst opts = def["options"];
        if (!opts.isNull()) {
            for (size_t i = 0; i < opts.size() && i < MAX_OPTIONS_PER_ITEM; i++) {
                strncpy(item->options[i], opts[i] | "", MAX_OPTION_LEN - 1);
                item->optionCount++;
            }
        }
        return item;
    }
    if (strcmp(type, "numeric") == 0) {
        NumericItem* item = new NumericItem();
        strncpy(item->_name, name, MAX_NAME_LEN - 1);
        strncpy(item->_prompt, prompt, MAX_PROMPT_LEN - 1);
        item->minVal = def["min_val"] | (def["min"] | (int16_t)0);
        item->maxVal = def["max_val"] | (def["max"] | (int16_t)99);
        return item;
    }

    TextItem* item = new TextItem();
    strncpy(item->_name, name, MAX_NAME_LEN - 1);
    strncpy(item->_prompt, prompt, MAX_PROMPT_LEN - 1);
    return item;
}

void LikertItem::build(lv_obj_t* parent) {
    startTimer();

    lv_obj_t* scaleRow = lv_obj_create(parent);
    lv_obj_set_width(scaleRow, 224);
    lv_obj_set_height(scaleRow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(scaleRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scaleRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(scaleRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(scaleRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_column(scaleRow, 4, 0);
    lv_obj_set_style_pad_all(scaleRow, 2, 0);
    lv_obj_set_style_pad_top(scaleRow, 0, 0);
    lv_obj_set_style_pad_bottom(scaleRow, 0, 0);
    lv_obj_set_style_margin_top(scaleRow, LIKERT_TOP_SPACING_PX, 0);

    int btnSize = (220 - (points - 1) * 6) / points;
    if (btnSize < 32) btnSize = 32;
    if (btnSize > 44) btnSize = 44;

    for (uint8_t i = 1; i <= points; i++) {
        lv_obj_t* btn = lv_button_create(scaleRow);
        lv_obj_set_size(btn, btnSize, btnSize);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), LV_STATE_FOCUSED);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(btn, 2, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_ACCENT), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_ACCENT), LV_STATE_FOCUSED);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, selectionCb, LV_EVENT_CLICKED, this);
        lv_obj_t* lbl = lv_label_create(btn);
        char buf[4]; snprintf(buf, sizeof(buf), "%d", i);
        lv_label_set_text(lbl, buf);
        lv_obj_set_style_text_color(lbl, lv_color_hex(COLOR_TEXT), 0);
        lv_obj_set_style_text_font(lbl, &font_de_14, 0);
        lv_obj_center(lbl);
    }

    lv_obj_t* labelRow = lv_obj_create(parent);
    lv_obj_set_size(labelRow, 224, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(labelRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(labelRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(labelRow, 0, 0);
    lv_obj_set_flex_flow(labelRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(labelRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* low = lv_label_create(labelRow);
    lv_label_set_text(low, leftLabel);
    lv_obj_set_width(low, 100);
    lv_label_set_long_mode(low, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(low, lv_color_hex(0x555566), 0);
    lv_obj_set_style_text_font(low, &font_de_14, 0);
    lv_obj_set_style_text_align(low, LV_TEXT_ALIGN_LEFT, 0);

    lv_obj_t* high = lv_label_create(labelRow);
    lv_label_set_text(high, rightLabel);
    lv_obj_set_width(high, 100);
    lv_label_set_long_mode(high, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(high, lv_color_hex(0x555566), 0);
    lv_obj_set_style_text_font(high, &font_de_14, 0);
    lv_obj_set_style_text_align(high, LV_TEXT_ALIGN_RIGHT, 0);

    addContinueButton(parent, true);
}

void LikertItem::highlight(lv_obj_t* row) {
    uint32_t cnt = lv_obj_get_child_count(row);
    for (uint32_t i = 0; i < cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(row, i);
        if (!lv_obj_check_type(child, &lv_button_class)) continue;
        int16_t cv = (int16_t)(intptr_t)lv_obj_get_user_data(child);
        bool sel = (cv == _value);
        lv_obj_set_style_bg_color(child, lv_color_hex(sel ? COLOR_ACCENT : 0xFFFFFF), 0);
        lv_obj_set_style_bg_color(child, lv_color_hex(sel ? COLOR_ACCENT : 0xFFFFFF), LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(child, lv_color_hex(sel ? COLOR_ACCENT : 0xFFFFFF), LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(child, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_t* l = lv_obj_get_child(child, 0);
        if (l) lv_obj_set_style_text_color(l, lv_color_hex(sel ? 0x000000 : COLOR_TEXT), 0);
    }
}

void VASItem::build(lv_obj_t* parent) {
    startTimer();

    _slider = lv_slider_create(parent);
    lv_obj_set_width(_slider, 200);
    lv_obj_set_style_margin_top(_slider, VAS_TOP_SPACING_PX, 0);
    lv_slider_set_range(_slider, 0, 100);
    lv_slider_set_value(_slider, 50, LV_ANIM_OFF);
    applySliderStyle(_slider);
    lv_obj_set_user_data(_slider, this);
    lv_obj_add_event_cb(_slider, sliderCb, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t* labelRow = lv_obj_create(parent);
    lv_obj_set_size(labelRow, 224, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(labelRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(labelRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(labelRow, 0, 0);

    lv_obj_set_flex_flow(labelRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(labelRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* low = lv_label_create(labelRow);
    lv_label_set_text(low, leftLabel);
    lv_obj_set_width(low, 100);
    lv_label_set_long_mode(low, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(low, lv_color_hex(0x555566), 0);
    lv_obj_set_style_text_font(low, &font_de_14, 0);
    lv_obj_set_style_text_align(low, LV_TEXT_ALIGN_LEFT, 0);

    lv_obj_t* high = lv_label_create(labelRow);
    lv_label_set_text(high, rightLabel);
    lv_obj_set_width(high, 100);
    lv_label_set_long_mode(high, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(high, lv_color_hex(0x555566), 0);
    lv_obj_set_style_text_font(high, &font_de_14, 0);
    lv_obj_set_style_text_align(high, LV_TEXT_ALIGN_RIGHT, 0);

    addContinueButton(parent, true);
}

void VASItem::sliderCb(lv_event_t* e) {
    VASItem* item = (VASItem*)lv_event_get_user_data(e);
    if (!item) return;
    item->_value = lv_slider_get_value(item->_slider);
    item->_valueSet = true;
    item->enableContinue();
}

void ChoiceItem::build(lv_obj_t* parent) {
    startTimer();
    const int maxListH = 146;
    const lv_font_t* rowFont = &font_de_14;

    lv_obj_t* listBox = lv_obj_create(parent);
    lv_obj_set_size(listBox, 220, LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(listBox, maxListH, 0);
    lv_obj_set_style_margin_top(listBox, 0, 0);
    lv_obj_set_style_bg_opa(listBox, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(listBox, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(listBox, 0, 0);
    lv_obj_set_style_pad_row(listBox, 0, 0);
    lv_obj_set_flex_flow(listBox, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(listBox, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(listBox, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(listBox, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_width(listBox, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(listBox, lv_color_hex(COLOR_ACCENT), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(listBox, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_style_radius(listBox, 2, LV_PART_SCROLLBAR);

    for (uint8_t i = 0; i < optionCount; i++) {
        lv_obj_t* row = lv_obj_create(listBox);
        lv_obj_set_size(row, 216, LV_SIZE_CONTENT);
        lv_obj_set_style_min_height(row, 30, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_left(row, 4, 0);
        lv_obj_set_style_pad_right(row, 4, 0);
        lv_obj_set_style_pad_row(row, 0, 0);
        lv_obj_set_style_pad_column(row, 10, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_set_user_data(row, (void*)(intptr_t)(i + 1));
        lv_obj_add_event_cb(row, rowClickedCb, LV_EVENT_CLICKED, this);

        int boxSize = 22;
        lv_obj_t* box = lv_obj_create(row);
        lv_obj_set_size(box, boxSize, boxSize);
        lv_obj_set_size(box, boxSize, boxSize);
        lv_obj_set_style_radius(box, 4, 0);
        lv_obj_set_style_bg_color(box, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(box, 2, 0);
        lv_obj_set_style_border_color(box, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_border_opa(box, LV_OPA_COVER, 0);
        lv_obj_set_style_pad_all(box, 0, 0);
        lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(box, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t* check = lv_label_create(box);
        lv_label_set_text(check, LV_SYMBOL_OK);
        lv_obj_set_style_text_color(check, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(check, &font_de_14, 0);
        lv_obj_center(check);
        lv_obj_add_flag(check, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t* lbl = lv_label_create(row);
        lv_label_set_text(lbl, options[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x1A1A2E), 0);
        lv_obj_set_style_text_font(lbl, rowFont, 0);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(lbl, 220 - 24 - 14);
        lv_obj_set_flex_grow(lbl, 1);
    }

    addContinueButton(parent, true);
}

void ChoiceItem::rowClickedCb(lv_event_t* e) {
    ChoiceItem* item = (ChoiceItem*)lv_event_get_user_data(e);
    lv_obj_t* row = (lv_obj_t*)lv_event_get_current_target(e);
    if (!item || !row) return;
    uint8_t idx = (uint8_t)(intptr_t)lv_obj_get_user_data(row);
    if (idx == 0) return;
    item->onRowClicked(idx, row);
}

void ChoiceItem::onRowClicked(uint8_t idx, lv_obj_t* row) {

    _value = (int16_t)idx;
    _valueSet = true;
    enableContinue();

    lv_obj_t* parent = lv_obj_get_parent(row);
    uint32_t cnt = lv_obj_get_child_count(parent);
    for (uint32_t i = 0; i < cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        if (lv_obj_get_user_data(child) == nullptr) continue;
        setRowChecked(child, child == row);
    }
}

void ChoiceItem::setRowChecked(lv_obj_t* row, bool checked) {
    if (!row || lv_obj_get_child_count(row) < 1) return;
    lv_obj_t* box = lv_obj_get_child(row, 0);
    if (!box) return;
    lv_obj_set_style_bg_color(box, lv_color_hex(checked ? COLOR_ACCENT : 0xFFFFFF), 0);
    if (lv_obj_get_child_count(box) >= 1) {
        lv_obj_t* check = lv_obj_get_child(box, 0);
        if (check) {
            if (checked) lv_obj_remove_flag(check, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(check, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void MultiChoiceItem::onRowClicked(uint8_t idx, lv_obj_t* row) {
    uint16_t mask = (uint16_t)_value;
    uint16_t bit = (uint16_t)(1U << (idx - 1));
    bool nowChecked = !(mask & bit);
    if (nowChecked) mask |= bit; else mask &= ~bit;
    _value = (int16_t)mask;
    _valueSet = (mask != 0);

    setRowChecked(row, nowChecked);

    if (_valueSet) enableContinue();
    else if (_continueBtn) {
        lv_obj_add_state(_continueBtn, LV_STATE_DISABLED);
    }
}

void NumericItem::build(lv_obj_t* parent) {
    startTimer();
    _value = minVal;
    _valueSet = true;

    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, 160, 44);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_column(row, 10, 0);
    lv_obj_set_style_pad_all(row, 0, 0);

    lv_obj_t* decBtn = lv_button_create(row);
    lv_obj_set_size(decBtn, 36, 36);
    lv_obj_add_style(decBtn, &styleBtn, 0);
    lv_obj_add_event_cb(decBtn, decCb, LV_EVENT_CLICKED, this);
    lv_obj_t* dl = lv_label_create(decBtn);
    lv_label_set_text(dl, LV_SYMBOL_MINUS);
    lv_obj_center(dl);

    _valueLbl = lv_label_create(row);
    lv_obj_set_style_text_color(_valueLbl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_text_font(_valueLbl, &font_de_20, 0);
    updateLabel();

    lv_obj_t* incBtn = lv_button_create(row);
    lv_obj_set_size(incBtn, 36, 36);
    lv_obj_add_style(incBtn, &styleBtn, 0);
    lv_obj_add_event_cb(incBtn, incCb, LV_EVENT_CLICKED, this);
    lv_obj_t* il = lv_label_create(incBtn);
    lv_label_set_text(il, LV_SYMBOL_PLUS);
    lv_obj_center(il);

    addContinueButton(parent, false);
}

void NumericItem::updateLabel() {
    if (_valueLbl) {
        char buf[8]; snprintf(buf, sizeof(buf), "%d", _value);
        lv_label_set_text(_valueLbl, buf);
    }
}

void NumericItem::incCb(lv_event_t* e) {
    NumericItem* item = (NumericItem*)lv_event_get_user_data(e);
    if (item && item->_value < item->maxVal) {
        item->_value++;
        item->updateLabel();
    }
}

void NumericItem::decCb(lv_event_t* e) {
    NumericItem* item = (NumericItem*)lv_event_get_user_data(e);
    if (item && item->_value > item->minVal) {
        item->_value--;
        item->updateLabel();
    }
}

void TextItem::build(lv_obj_t* parent) {
    startTimer();
    _valueSet = true;
    _value = 0;
    addContinueButton(parent, false);
}

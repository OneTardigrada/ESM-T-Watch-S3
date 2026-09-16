#include "ui/Styles.h"
#include "fonts_de.h"

bool g_darkMode = true;
uint8_t g_wallpaper = 0;

lv_style_t styleContainer;
lv_style_t styleTitle;
lv_style_t stylePrompt;
lv_style_t styleBtn;
lv_style_t styleBtnDisabled;

static bool _initialized = false;

static void applyColors() {
    lv_style_set_bg_color(&styleContainer, lv_color_hex(colorBg()));

    lv_style_set_text_color(&styleTitle, lv_color_hex(COLOR_ACCENT));

    lv_style_set_text_color(&stylePrompt, lv_color_hex(colorText()));

    lv_style_set_bg_color(&styleBtn, lv_color_hex(COLOR_ACCENT));
    lv_style_set_text_color(&styleBtn, lv_color_hex(0x000000));
}

void stylesInit() {
    if (_initialized) return;

    lv_style_init(&styleContainer);
    lv_style_set_bg_opa(&styleContainer, LV_OPA_COVER);
    lv_style_set_pad_all(&styleContainer, 0);

    lv_style_init(&styleTitle);
    lv_style_set_text_font(&styleTitle, &font_de_20);

    lv_style_init(&stylePrompt);
    lv_style_set_text_font(&stylePrompt, &font_de_14);

    lv_style_init(&styleBtn);
    lv_style_set_text_font(&styleBtn, &font_de_14);
    lv_style_set_radius(&styleBtn, 10);
    lv_style_set_pad_ver(&styleBtn, 6);
    lv_style_set_pad_hor(&styleBtn, 12);

    lv_style_init(&styleBtnDisabled);
    lv_style_set_bg_color(&styleBtnDisabled, lv_color_hex(0x444444));
    lv_style_set_text_color(&styleBtnDisabled, lv_color_hex(0x888888));

    applyColors();
    _initialized = true;
}

void stylesReinit() {
    applyColors();
}

static uint32_t wallpaperLineColor() {

    return g_darkMode ? 0x2A2A45 : 0xD8D8E0;
}

static void drawGrid(lv_obj_t* parent) {

    const uint32_t color = wallpaperLineColor();
    for (int y = 20; y <= 220; y += 40) {
        for (int x = 20; x <= 220; x += 40) {
            lv_obj_t* dot = lv_obj_create(parent);
            lv_obj_remove_style_all(dot);
            lv_obj_set_size(dot, 3, 3);
            lv_obj_set_pos(dot, x, y);
            lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(dot, lv_color_hex(color), 0);
            lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
            lv_obj_clear_flag(dot, (lv_obj_flag_t)(LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));
        }
    }
}

static void drawArcs(lv_obj_t* parent) {

    const uint32_t color = wallpaperLineColor();
    const int sizes[] = { 200, 150, 100 };
    for (int s : sizes) {
        lv_obj_t* ring = lv_obj_create(parent);
        lv_obj_remove_style_all(ring);
        lv_obj_set_size(ring, s, s);
        lv_obj_align(ring, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(ring, 1, 0);
        lv_obj_set_style_border_color(ring, lv_color_hex(color), 0);
        lv_obj_set_style_border_opa(ring, LV_OPA_60, 0);
        lv_obj_clear_flag(ring, (lv_obj_flag_t)(LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));
    }
}

void drawWallpaper(lv_obj_t* parent) {
    switch (g_wallpaper) {
        case 1: drawGrid(parent); break;
        case 2: drawArcs(parent); break;
        case 0:
        default: break;
    }
}

lv_obj_t* createCancelX(lv_obj_t* parent, lv_event_cb_t cb, void* userData) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 36, 36);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xC2185B), 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -14, 8);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, userData);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lbl, &font_de_14, 0);
    lv_obj_center(lbl);
    return btn;
}

void resetScreen(lv_obj_t* scr) {
    lv_obj_clean(scr);
    lv_obj_add_style(scr, &styleContainer, 0);
}

lv_obj_t* createStyledLabel(lv_obj_t* parent, const char* text, uint32_t color,
                             const lv_font_t* font, lv_align_t align,
                             lv_coord_t x, lv_coord_t y) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(lbl, font, 0);
    if (align != LV_ALIGN_DEFAULT) lv_obj_align(lbl, align, x, y);
    return lbl;
}

lv_obj_t* createAccentButton(lv_obj_t* parent, int w, int h, int radius,
                              const char* labelText, const lv_font_t* labelFont,
                              lv_event_cb_t cb, void* userData) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_ACCENT), 0);
    lv_obj_set_style_radius(btn, radius, 0);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_hex(COLOR_ACCENT_DARK), 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_50, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, userData);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, labelText);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl, labelFont, 0);
    lv_obj_center(lbl);
    return btn;
}

void applySliderStyle(lv_obj_t* slider) {
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_top(slider, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(slider, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(slider, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(slider, lv_color_hex(COLOR_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_border_opa(slider, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_ACCENT_DARK), LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 8, LV_PART_KNOB);
    lv_obj_set_style_border_width(slider, 3, LV_PART_KNOB);
    lv_obj_set_style_border_color(slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_shadow_width(slider, 8, LV_PART_KNOB);
    lv_obj_set_style_shadow_color(slider, lv_color_hex(COLOR_ACCENT_DARK), LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(slider, LV_OPA_40, LV_PART_KNOB);
}

lv_obj_t* createNumpadButton(lv_obj_t* parent, const char* text) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 54, 32);
    lv_obj_set_style_bg_color(btn, lv_color_hex(colorBtnBg()), 0);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(colorText()), 0);
    lv_obj_set_style_text_font(lbl, &font_de_14, 0);
    lv_obj_center(lbl);
    return btn;
}

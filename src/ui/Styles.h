#pragma once

#include <lvgl.h>

#define COLOR_ACCENT      0x64B5F6
#define COLOR_ACCENT_DARK 0x1976D2
#define COLOR_DANGER       0x882222
#define COLOR_WARNING      0xFFAA00
#define COLOR_ERROR        0xFF5555

extern bool g_darkMode;

extern uint8_t g_wallpaper;
constexpr uint8_t WALLPAPER_COUNT = 3;

void drawWallpaper(lv_obj_t* parent);

inline uint32_t colorBg()        { return g_darkMode ? 0x1A1A2E : 0xEEEEF0; }
inline uint32_t colorBtnBg()     { return g_darkMode ? 0x2A2A45 : 0xCCCCDD; }
inline uint32_t colorBtnBorder() { return g_darkMode ? 0x3D3D5C : 0xAAAABB; }
inline uint32_t colorText()      { return g_darkMode ? 0xFFFFFF : 0x1A1A2E; }
inline uint32_t colorTextDim()   { return g_darkMode ? 0xAAAAAA : 0x666666; }
inline uint32_t colorStatusBar() { return g_darkMode ? 0x111122 : 0xDDDDEE; }
inline uint32_t colorBtnText()   { return g_darkMode ? 0x000000 : 0x000000; }

#define COLOR_BG          colorBg()
#define COLOR_BTN_BG      colorBtnBg()
#define COLOR_BTN_BORDER  colorBtnBorder()
#define COLOR_TEXT         colorText()
#define COLOR_TEXT_DIM     colorTextDim()
#define COLOR_STATUS_BAR   colorStatusBar()

extern lv_style_t styleContainer;
extern lv_style_t styleTitle;
extern lv_style_t stylePrompt;
extern lv_style_t styleBtn;
extern lv_style_t styleBtnDisabled;

void stylesInit();
void stylesReinit();

lv_obj_t* createCancelX(lv_obj_t* parent, lv_event_cb_t cb, void* userData);

void resetScreen(lv_obj_t* scr);

lv_obj_t* createStyledLabel(lv_obj_t* parent, const char* text, uint32_t color,
                             const lv_font_t* font, lv_align_t align,
                             lv_coord_t x, lv_coord_t y);

lv_obj_t* createAccentButton(lv_obj_t* parent, int w, int h, int radius,
                              const char* labelText, const lv_font_t* labelFont,
                              lv_event_cb_t cb, void* userData);

void applySliderStyle(lv_obj_t* slider);

lv_obj_t* createNumpadButton(lv_obj_t* parent, const char* text);

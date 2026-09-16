// TFT_eSPI User Setup für LilyGO T-Watch S3
// Diese Datei überschreibt die Standard-Konfiguration

#ifndef USER_SETUP_H
#define USER_SETUP_H

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 280

#define TFT_MISO -1
#define TFT_MOSI 11
#define TFT_SCLK 10
#define TFT_CS   12
#define TFT_DC   13
#define TFT_RST  14
#define TFT_BL   15

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8

#define SMOOTH_FONT

#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000

#define SPI_TOUCH_FREQUENCY  2500000

#endif
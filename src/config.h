#pragma once

#include <Arduino.h>

#define DISPLAY_WIDTH    240
#define DISPLAY_HEIGHT   240

#define SERIAL_BAUD      115200

#define CONFIG_FILE      "/config.json"
#define DATA_FILE        "/data.enc"
#define KEY_FILE         "/key.bin"

#define DEFAULT_DISPLAY_TIMEOUT_MS  15000
#define NOTIF_DISPLAY_TIMEOUT_MS    15000
#define DEFAULT_BRIGHTNESS          100

#define USB_MODE_AUTOOFF_MS         (30UL * 60UL * 1000UL)

#define AUTH_SESSION_TIMEOUT_MS     60000
#define MAX_PIN_LENGTH              8
#define AES_KEY_BYTES               32
#define AES_IV_BYTES                16
#define AES_BLOCK_SIZE              16

#define CONFIG_VERSION              2

#define MAX_QUESTIONNAIRES          5
#define MAX_ITEMS_PER_QUEST         20
#define MAX_OPTIONS_PER_ITEM        8
#define MAX_NOTIFICATIONS           10
#define MAX_NAME_LEN                32
#define MAX_LABEL_LEN               32
#define MAX_PROMPT_LEN              256
#define MAX_OPTION_LEN              64

enum ItemType : uint8_t {
    ITEM_TEXT,
    ITEM_LIKERT,
    ITEM_VAS,
    ITEM_OPTIONS,
    ITEM_NUMERIC
};

#define LIKERT_TOP_SPACING_PX      12
#define VAS_TOP_SPACING_PX         12

#define DEFAULT_REMINDER_AFTER_MIN         30
#define DEFAULT_EXPIRY_MIN                 10
#define DEFAULT_EXPIRY_AFTER_REMINDER_MIN  10
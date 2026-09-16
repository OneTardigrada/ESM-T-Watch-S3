// ConfigManager — JSON-Konfiguration (LittleFS + ArduinoJson v7)
#include "services/ConfigManager.h"
#include <LittleFS.h>

static const char DEFAULT_CONFIG[] PROGMEM = R"JSON({
  "config_version": 2,
  "subject_id": "DEFAULT",
  "pin": "1234",
  "study_name": "ESM_Studie",
  "questionnaires": [
    {
      "name": "ESM_Scheduled",
      "event": false,
      "reminder_after_min": 30,
      "expiry_min": 10,
      "expiry_after_reminder_min": 10,
      "notifications": [
        {"hour": 9, "minute": 0},
        {"hour": 13, "minute": 0},
        {"hour": 17, "minute": 0},
        {"hour": 21, "minute": 0}
      ],
      "items": [
        {"name": "unreported_cravings", "type": "numeric", "prompt": "Wie viele Cravings hatten\nSie in den letzten\n4 Stunden, die Sie\nnicht berichtet haben?", "min_val": 0, "max_val": 99},
        {"name": "craving_followed", "type": "likert", "prompt": "Sind Sie Ihrem\nCraving nachgegangen?", "scale_min": 1, "scale_max": 5, "min_label": "nie", "max_label": "immer"},
        {"name": "food_eaten", "type": "options", "prompt": "Was haben Sie gegessen?", "options": ["... etwas S\u00fc\u00dfem", "... etwas Herzhaftem", "... einem salzigen Snack", "... Obst/Gem\u00fcse"]}
      ]
    },
    {
      "name": "Craving",
      "event": true,
      "items": [
        {"name": "craving_type", "type": "options", "prompt": "Ich habe ein Craving nach ...", "options": ["... etwas S\u00fc\u00dfem", "... etwas Herzhaftem", "... einem salzigen Snack", "... Obst/Gem\u00fcse"]},
        {"name": "craving_content", "type": "options", "prompt": "Mein Craving enth\u00e4lt ...", "options": ["... Fleisch oder Fleischprodukte", "... Eier oder Milchprodukte", "... H\u00fclsenfr\u00fcchte, Fleischersatz oder N\u00fcsse", "... Nichts davon"]}
      ]
    }
  ]
})JSON";

bool ConfigManager::loadFromFile() {
    if (!LittleFS.exists(CONFIG_FILE)) return false;

    File f = LittleFS.open(CONFIG_FILE, "r");
    if (!f) return false;

    _doc.clear();
    DeserializationError err = deserializeJson(_doc, f);
    f.close();

    if (err) {
        Serial.printf("Config-Parse-Fehler: %s\n", err.c_str());
        return false;
    }

    _loaded = true;
    return true;
}

bool ConfigManager::parseAndStore(const String& json) {
    _doc.clear();
    DeserializationError err = deserializeJson(_doc, json);
    if (err) {
        Serial.printf("JSON-Fehler: %s\n", err.c_str());
        return false;
    }

    if (!_doc["subject_id"].is<const char*>() ||
        !_doc["pin"].is<const char*>()) {
        Serial.println("Config: subject_id und pin sind Pflichtfelder");
        return false;
    }

    if (!saveToFile(json)) return false;

    _loaded = true;
    return true;
}

bool ConfigManager::saveToFile(const String& json) {
    File f = LittleFS.open(CONFIG_FILE, "w");
    if (!f) return false;
    f.print(json);
    f.close();
    return true;
}

bool ConfigManager::writeDefaultConfig() {
    String json(DEFAULT_CONFIG);
    return parseAndStore(json);
}

const char* ConfigManager::subjectId() const {
    return _loaded ? (_doc["subject_id"] | "UNKNOWN") : "UNKNOWN";
}

const char* ConfigManager::studyName() const {
    return _loaded ? (_doc["study_name"] | "ESM_Study") : "ESM_Study";
}

const char* ConfigManager::pin() const {
    return _loaded ? (_doc["pin"] | "0000") : "0000";
}

uint8_t ConfigManager::questionnaireCount() const {
    if (!_loaded) return 0;
    JsonArrayConst arr = _doc["questionnaires"];
    return arr.isNull() ? 0 : (uint8_t)arr.size();
}

JsonObjectConst ConfigManager::getQuestionnaire(uint8_t index) const {
    if (!_loaded) return JsonObjectConst();
    JsonArrayConst arr = _doc["questionnaires"];
    if (arr.isNull() || index >= arr.size()) return JsonObjectConst();
    return arr[index];
}

JsonObjectConst ConfigManager::getQuestionnaireByName(const char* name) const {
    if (!_loaded) return JsonObjectConst();
    JsonArrayConst arr = _doc["questionnaires"];
    if (arr.isNull()) return JsonObjectConst();
    for (JsonObjectConst q : arr) {
        if (strcmp(q["name"] | "", name) == 0) return q;
    }
    return JsonObjectConst();
}

uint8_t ConfigManager::eventQuestionnaireCount() const {
    if (!_loaded) return 0;
    JsonArrayConst arr = _doc["questionnaires"];
    if (arr.isNull()) return 0;
    uint8_t count = 0;
    for (JsonObjectConst q : arr) {
        if (q["event"] | false) count++;
    }
    return count;
}

JsonObjectConst ConfigManager::getEventQuestionnaire(uint8_t index) const {
    if (!_loaded) return JsonObjectConst();
    JsonArrayConst arr = _doc["questionnaires"];
    if (arr.isNull()) return JsonObjectConst();
    uint8_t count = 0;
    for (JsonObjectConst q : arr) {
        if (q["event"] | false) {
            if (count == index) return q;
            count++;
        }
    }
    return JsonObjectConst();
}

uint8_t ConfigManager::configVersion() const {
    return _loaded ? (_doc["config_version"] | 0) : 0;
}

JsonObjectConst ConfigManager::root() const {
    return _doc.as<JsonObjectConst>();
}

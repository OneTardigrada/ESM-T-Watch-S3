#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

class ConfigManager {
public:
    bool loadFromFile();
    bool parseAndStore(const String& json);

    bool writeDefaultConfig();

    bool isLoaded() const { return _loaded; }

    const char* subjectId()    const;
    const char* studyName()    const;
    const char* pin()          const;

    uint8_t questionnaireCount() const;

    JsonObjectConst getQuestionnaire(uint8_t index) const;

    JsonObjectConst getQuestionnaireByName(const char* name) const;

    uint8_t eventQuestionnaireCount() const;
    JsonObjectConst getEventQuestionnaire(uint8_t index) const;

    uint8_t configVersion() const;

    JsonObjectConst root() const;

private:
    JsonDocument _doc;
    bool _loaded = false;

    bool saveToFile(const String& json);
};

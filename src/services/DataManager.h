#pragma once

#include <Arduino.h>
#include "config.h"

class CryptoManager;

class DataManager {
public:
    explicit DataManager(CryptoManager& crypto) : _crypto(crypto) {}

    bool begin();

    bool saveResponse(const char* timestamp,
                      const char* subjectId,
                      const char* questName,
                      const char* itemName,
                      const char* itemType,
                      const char* prompt,
                      int16_t response,
                      unsigned long durationMs);

    void exportSerial();

    bool deleteAll();

    uint16_t rowCount();

    uint16_t countCompleted(const char* questName, const char* lastItemName);

private:
    CryptoManager& _crypto;
};

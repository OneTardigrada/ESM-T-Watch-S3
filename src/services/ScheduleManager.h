#pragma once

#include <Arduino.h>
#include "config.h"

class ConfigManager;

struct AlarmEntry {
    char questName[MAX_NAME_LEN];
    uint8_t hour;
    uint8_t minute;
    uint8_t reminderMin;
    uint8_t expiryMin;
    uint8_t expiryAfterReminderMin;
    uint8_t lastTriggeredDay;
    bool    isActive;
    bool    isDismissed;
    bool    reminderWasSent;
    bool    answered;
    int16_t firstShownMin;
    int16_t reminderShownMin;
};

class ScheduleManager {
public:
    explicit ScheduleManager(ConfigManager& config) : _config(config) {}

    void loadFromConfig();

    int8_t checkAlarms(uint8_t curDay, uint8_t curHour, uint8_t curMinute, bool* isFreshTrigger = nullptr);

    bool markHandled(int8_t alarmIdx);

    void markAnswered(int8_t alarmIdx);

    uint32_t secondsToNextAlarm(uint8_t curHour, uint8_t curMinute) const;

    const char* alarmQuestName(int8_t idx) const;

    int8_t pendingAlarmIndex() const;

    bool consumeAutoExpired(char* expQuest, size_t bufLen);

    void reset();

    uint8_t alarmCount() const { return _count; }

private:
    void loadPersistedState();
    void savePersistedState(uint8_t idx);
    void clearPersistedState();

    ConfigManager& _config;
    AlarmEntry _alarms[MAX_NOTIFICATIONS * MAX_QUESTIONNAIRES] = {};
    uint8_t _count = 0;
    char    _autoExpiredQuest[MAX_NAME_LEN] = {};
    bool    _hasAutoExpired = false;
};

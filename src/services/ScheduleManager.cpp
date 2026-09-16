// ScheduleManager — ESM-Zeitplan aus JSON-Config
#include "services/ScheduleManager.h"
#include "services/ConfigManager.h"
#include <Preferences.h>

static const char* PREFS_NS = "sched";

static void prefsKey(char* buf, size_t bufLen, const char* prefix, uint8_t idx) {
    snprintf(buf, bufLen, "%s%u", prefix, (unsigned)idx);
}

void ScheduleManager::loadPersistedState() {
    Preferences p;
    if (!p.begin(PREFS_NS, true)) return;
    char k[16];
    for (uint8_t i = 0; i < _count; i++) {
        AlarmEntry& a = _alarms[i];
        prefsKey(k, sizeof(k), "tg", i); a.lastTriggeredDay = p.getUChar(k, 0);
        prefsKey(k, sizeof(k), "an", i); a.answered         = p.getBool(k, false);
        prefsKey(k, sizeof(k), "dm", i); a.isDismissed      = p.getBool(k, false);
        prefsKey(k, sizeof(k), "rm", i); a.reminderWasSent  = p.getBool(k, false);
        prefsKey(k, sizeof(k), "ac", i); a.isActive         = p.getBool(k, false);
        prefsKey(k, sizeof(k), "fs", i); a.firstShownMin    = p.getShort(k, -1);
        prefsKey(k, sizeof(k), "rs", i); a.reminderShownMin = p.getShort(k, -1);
    }
    p.end();
}

void ScheduleManager::savePersistedState(uint8_t idx) {
    if (idx >= _count) return;
    Preferences p;
    if (!p.begin(PREFS_NS, false)) return;
    const AlarmEntry& a = _alarms[idx];
    char k[16];
    prefsKey(k, sizeof(k), "tg", idx); p.putUChar(k, a.lastTriggeredDay);
    prefsKey(k, sizeof(k), "an", idx); p.putBool(k, a.answered);
    prefsKey(k, sizeof(k), "dm", idx); p.putBool(k, a.isDismissed);
    prefsKey(k, sizeof(k), "rm", idx); p.putBool(k, a.reminderWasSent);
    prefsKey(k, sizeof(k), "ac", idx); p.putBool(k, a.isActive);
    prefsKey(k, sizeof(k), "fs", idx); p.putShort(k, a.firstShownMin);
    prefsKey(k, sizeof(k), "rs", idx); p.putShort(k, a.reminderShownMin);
    p.end();
}

void ScheduleManager::clearPersistedState() {
    Preferences p;
    if (!p.begin(PREFS_NS, false)) return;
    p.clear();
    p.end();
}

void ScheduleManager::loadFromConfig() {
    _count = 0;
    uint8_t qCount = _config.questionnaireCount();

    for (uint8_t qi = 0; qi < qCount && _count < sizeof(_alarms)/sizeof(_alarms[0]); qi++) {
        JsonObjectConst quest = _config.getQuestionnaire(qi);
        if (quest.isNull()) continue;
        if (quest["event"] | false) continue;

        const char* name = quest["name"] | "";
        JsonArrayConst notifs = quest["notifications"];
        if (notifs.isNull()) continue;

        uint8_t reminderMin = quest["reminder_after_min"] | DEFAULT_REMINDER_AFTER_MIN;
        uint8_t expiryMin   = quest["expiry_min"] | DEFAULT_EXPIRY_MIN;
        uint8_t expiryAfterReminderMin = quest["expiry_after_reminder_min"] | DEFAULT_EXPIRY_AFTER_REMINDER_MIN;

        for (JsonObjectConst n : notifs) {
            if (_count >= sizeof(_alarms)/sizeof(_alarms[0])) break;

            AlarmEntry& a = _alarms[_count];
            strncpy(a.questName, name, MAX_NAME_LEN - 1);
            a.questName[MAX_NAME_LEN - 1] = '\0';
            a.hour = n["hour"] | 0;
            a.minute = n["minute"] | 0;
            a.reminderMin = reminderMin;
            a.expiryMin = expiryMin;
            a.expiryAfterReminderMin = expiryAfterReminderMin;
            a.lastTriggeredDay = 0;
            a.isActive = false;
            a.isDismissed = false;
            a.reminderWasSent = false;
            a.answered = false;
            a.firstShownMin = -1;
            a.reminderShownMin = -1;
            _count++;
        }
    }

    loadPersistedState();
    Serial.printf("ScheduleManager: %d Alarme geladen\n", _count);
}

int8_t ScheduleManager::checkAlarms(uint8_t curDay, uint8_t curHour, uint8_t curMinute, bool* isFreshTrigger) {
    int nowMin = curHour * 60 + curMinute;
    if (isFreshTrigger) *isFreshTrigger = false;

    for (uint8_t i = 0; i < _count; i++) {
        AlarmEntry& a = _alarms[i];
        int alarmMin = a.hour * 60 + a.minute;
        int elapsed = nowMin - alarmMin;

        if (elapsed < 0) continue;

        if (elapsed >= 1440) continue;

        if (a.lastTriggeredDay != curDay) {
            a.lastTriggeredDay = curDay;
            a.isDismissed = false;
            a.reminderWasSent = false;
            a.answered = false;
            a.firstShownMin = -1;
            a.reminderShownMin = -1;

            if (elapsed > a.expiryMin) {
                a.isActive = false;
                savePersistedState(i);
                continue;
            }
            a.isActive = true;
            a.firstShownMin = nowMin;
            savePersistedState(i);
            if (isFreshTrigger) *isFreshTrigger = true;
            return i;
        }

        if (a.answered || a.isDismissed) continue;

        if (!a.reminderWasSent && a.firstShownMin >= 0 &&
            (nowMin - a.firstShownMin) >= a.reminderMin) {
            a.reminderWasSent = true;
            a.isActive = true;
            a.reminderShownMin = nowMin;
            savePersistedState(i);
            if (isFreshTrigger) *isFreshTrigger = true;
            return i;
        }

        if (a.reminderWasSent && a.reminderShownMin >= 0 &&
            (nowMin - a.reminderShownMin) >= a.expiryAfterReminderMin) {
            if (a.isActive) {
                strncpy(_autoExpiredQuest, a.questName, sizeof(_autoExpiredQuest) - 1);
                _autoExpiredQuest[sizeof(_autoExpiredQuest) - 1] = '\0';
                _hasAutoExpired = true;
            }
            a.isActive = false;
            a.isDismissed = true;
            savePersistedState(i);
            if (isFreshTrigger) *isFreshTrigger = false;
            continue;
        }

        if (a.isActive) {
            return i;
        }
    }
    return -1;
}

bool ScheduleManager::markHandled(int8_t alarmIdx) {
    if (alarmIdx < 0 || alarmIdx >= _count) return false;
    AlarmEntry& a = _alarms[alarmIdx];

    a.isActive = false;
    a.isDismissed = true;
    a.answered = false;
    savePersistedState(alarmIdx);
    return true;
}

void ScheduleManager::markAnswered(int8_t alarmIdx) {
    if (alarmIdx < 0 || alarmIdx >= _count) return;
    AlarmEntry& a = _alarms[alarmIdx];
    a.answered = true;
    a.isActive = false;
    savePersistedState(alarmIdx);
}

uint32_t ScheduleManager::secondsToNextAlarm(uint8_t curHour, uint8_t curMinute) const {
    int nowMin = curHour * 60 + curMinute;
    uint32_t minDelta = 86400;

    for (uint8_t i = 0; i < _count; i++) {
        if (_alarms[i].answered) continue;
        int alarmMin = _alarms[i].hour * 60 + _alarms[i].minute;
        int delta = alarmMin - nowMin;
        if (delta <= 0) delta += 1440;
        uint32_t secs = (uint32_t)delta * 60;
        if (secs < minDelta) minDelta = secs;
    }
    return minDelta;
}

const char* ScheduleManager::alarmQuestName(int8_t idx) const {
    if (idx < 0 || idx >= _count) return "";
    return _alarms[idx].questName;
}

void ScheduleManager::reset() {
    for (uint8_t i = 0; i < _count; i++) {
        _alarms[i].lastTriggeredDay = 0;
        _alarms[i].isActive = false;
        _alarms[i].isDismissed = false;
        _alarms[i].reminderWasSent = false;
        _alarms[i].answered = false;
        _alarms[i].firstShownMin = -1;
        _alarms[i].reminderShownMin = -1;
    }
    clearPersistedState();
}

int8_t ScheduleManager::pendingAlarmIndex() const {
    for (uint8_t i = 0; i < _count; i++) {
        const AlarmEntry& a = _alarms[i];
        if (a.isActive && !a.answered && !a.isDismissed) {
            return (int8_t)i;
        }
    }
    return -1;
}

bool ScheduleManager::consumeAutoExpired(char* expQuest, size_t bufLen) {
    if (!_hasAutoExpired) return false;
    if (expQuest && bufLen > 0) {
        strncpy(expQuest, _autoExpiredQuest, bufLen - 1);
        expQuest[bufLen - 1] = '\0';
    }
    _hasAutoExpired = false;
    _autoExpiredQuest[0] = '\0';
    return true;
}

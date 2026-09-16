#pragma once

#include <Arduino.h>
#include <LilyGoLib.h>
#include "config.h"
#include "services/ConfigManager.h"
#include "services/CryptoManager.h"
#include "services/DataManager.h"
#include "services/SerialProtocol.h"
#include "services/PowerManager.h"
#include "services/ScheduleManager.h"
#include "states/IState.h"

class App {
public:
    void setup();
    void loop();

    void requestState(StateType type, const char* context = nullptr);
    void startEventQuestionnaire(uint8_t index);
    void startScheduledQuestionnaire(const char* name);
    void showDone(const char* message);
    void showNotification(const char* questName, int8_t alarmIdx);

    void logNotAnswered(const char* questName);

    ConfigManager&   config()   { return _config; }
    CryptoManager&   crypto()   { return _crypto; }
    DataManager&     data()     { return _data; }
    PowerManager&    power()    { return _power; }
    ScheduleManager& schedule() { return _schedule; }

    void toggleTheme();
    void cycleWallpaper();

    void    setBrightness(uint8_t v);
    uint8_t brightness() const { return _brightness; }

    void setUsbMode(bool on);
    bool isUsbMode() const { return _usbMode; }

    uint8_t batteryPercent();
    bool    isCharging();
    void    vibrate(uint8_t effect = 15);

    void formatNow(char* buf, size_t len);

private:

    ConfigManager   _config;
    CryptoManager   _crypto;
    DataManager     _data{_crypto};
    SerialProtocol  _serial{this};
    PowerManager    _power{this};
    ScheduleManager _schedule{_config};

    IState*   _currentState = nullptr;
    StateType _nextStateType = StateType::Home;
    uint8_t   _brightness = DEFAULT_BRIGHTNESS;
    char      _nextStateContext[128] = {};
    bool      _stateChangeRequested = false;

    char    _pendingQuestName[MAX_NAME_LEN] = {};
    uint8_t _pendingQuestIdx = 0;
    int8_t  _pendingAlarmIdx = -1;
    bool    _pendingQuestScheduled = false;

    unsigned long _lastAlarmCheck = 0;
    unsigned long _lastDisplayUpdate = 0;
    unsigned long _lastSerialActivity = 0;

    bool _usbMode = false;
    bool _serialBusy = false;
    int8_t _lastCheckedMin = -1;

    uint8_t       _battCached = 0;
    unsigned long _battLastRead = 0;

    void initHardware();
    void enterNextState();
    void checkScheduledAlarms();

    void invalidateBattAndRedraw();

    static void onPmuEvent(DeviceEvent_t event, void* params, void* userData);
    void handlePmuButton(PMUEventType_t type);
};

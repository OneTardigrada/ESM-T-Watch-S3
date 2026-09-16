// =====================================================================
// App — Orchestrator-Implementierung
// =====================================================================
#include "App.h"

#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include "esp_bt.h"

#include "ui/Styles.h"

#include "states/HomeState.h"
#include "states/SettingsState.h"
#include "states/LockScreenState.h"
#include "states/QuestionnaireState.h"
#include "states/NotificationState.h"
#include "states/DoneState.h"

void App::setup() {

    setCpuFrequencyMhz(160);

    Serial.setRxBufferSize(8192);
    Serial.begin(SERIAL_BAUD);

    WiFi.mode(WIFI_OFF);
    btStop();
    esp_bt_controller_disable();

    Preferences prefs;
    prefs.begin("esm", true);
    g_darkMode = prefs.getBool("dark", true);
    g_wallpaper = prefs.getUChar("wp", 0);
    if (g_wallpaper >= WALLPAPER_COUNT) g_wallpaper = 0;
    _brightness = prefs.getUChar("bright", DEFAULT_BRIGHTNESS);
    if (_brightness < 8) _brightness = 8;
    prefs.end();

    initHardware();

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount FAILED!");
    }

    stylesInit();

    bool configOk = _config.loadFromFile();
    if (configOk && _config.configVersion() < CONFIG_VERSION) {
        Serial.printf("Config veraltet (v%d < v%d) — schreibe neue Default-Config...\n",
                       _config.configVersion(), CONFIG_VERSION);
        configOk = false;
    }
    if (configOk) {
        Serial.printf("Config OK (v%d) — Studie: %s  Subject: %s  Questionnaires: %d  Events: %d\n",
                       _config.configVersion(), _config.studyName(), _config.subjectId(),
                       _config.questionnaireCount(), _config.eventQuestionnaireCount());
    } else {
        Serial.println("Schreibe Default-Config...");
        if (_config.writeDefaultConfig()) {
            Serial.printf("Default-Config v%d geschrieben. Questionnaires: %d  Events: %d\n",
                           CONFIG_VERSION, _config.questionnaireCount(), _config.eventQuestionnaireCount());
        } else {
            Serial.println("FEHLER: Default-Config konnte nicht geschrieben werden!");
        }
    }
    _crypto.deriveKey(_config.pin());
    _schedule.loadFromConfig();

    _data.begin();

    _stateChangeRequested = true;
    _nextStateType = StateType::Home;
    enterNextState();

    _power.resetInactivityTimer();
    Serial.println("=== ESM Smartwatch bereit ===");
}

void App::loop() {
    instance.loop();
    if (_power.isDisplayOn()) lv_timer_handler();

    _serial.process();

    bool busy = (Serial.available() > 0);
    if (busy && !_serialBusy) {
        _power.resetInactivityTimer();
        _lastSerialActivity = millis();
    }
    _serialBusy = busy;

    if (instance.getTouched()) {
        _power.resetInactivityTimer();
        if (!_power.isDisplayOn()) _power.wakeDisplay();
    }

    if (_stateChangeRequested) {
        _stateChangeRequested = false;
        enterNextState();
    }

    if (_power.isDisplayOn() && _currentState) {
        if (millis() - _lastDisplayUpdate >= 2000) {
            _lastDisplayUpdate = millis();
            _currentState->update();
        }
    }

    if (_config.isLoaded() && millis() - _lastAlarmCheck >= 1000) {
        _lastAlarmCheck = millis();
        struct tm t; instance.rtc.getDateTime(&t);
        if (t.tm_min != _lastCheckedMin) {
            _lastCheckedMin = t.tm_min;
            checkScheduledAlarms();
        }
    }

    if (_usbMode) {
        unsigned long since = (_lastSerialActivity == 0) ? 0
                              : (millis() - _lastSerialActivity);

        if (since > USB_MODE_AUTOOFF_MS) {
            Serial.println("USB-Modus: Auto-Off (Timeout)");
            setUsbMode(false);
        }
    }

    if (_currentState && _currentState->type() == StateType::Home) {
        if (_power.isDisplayOn() && _power.isInactivityTimeout()) {

            if (_usbMode) {
                _power.resetInactivityTimer();
            } else {

                esp_sleep_wakeup_cause_t cause = _power.enterLightSleep();

                checkScheduledAlarms();

                if (cause == ESP_SLEEP_WAKEUP_TIMER) {

                    if (!_stateChangeRequested) {
                        return;
                    }
                }

                if (!_stateChangeRequested) {
                    _power.wakeDisplay();
                }
            }
        }
    }

    if (_power.isDisplayOn()) {
        delay(5);
    }
}

void App::requestState(StateType type, const char* context) {
    _nextStateType = type;
    memset(_nextStateContext, 0, sizeof(_nextStateContext));
    if (context) {
        strncpy(_nextStateContext, context, sizeof(_nextStateContext) - 1);
    }
    _stateChangeRequested = true;
}

void App::startEventQuestionnaire(uint8_t index) {
    _pendingQuestIdx = index;
    _pendingQuestScheduled = false;
    _nextStateType = StateType::Questionnaire;
    _stateChangeRequested = true;
}

void App::startScheduledQuestionnaire(const char* name) {
    memset(_pendingQuestName, 0, sizeof(_pendingQuestName));
    strncpy(_pendingQuestName, name, sizeof(_pendingQuestName) - 1);
    _pendingQuestScheduled = true;
    _nextStateType = StateType::Questionnaire;
    _stateChangeRequested = true;
}

void App::showDone(const char* message) {
    requestState(StateType::Done, message);
}

void App::showNotification(const char* questName, int8_t alarmIdx) {
    memset(_pendingQuestName, 0, sizeof(_pendingQuestName));
    strncpy(_pendingQuestName, questName, sizeof(_pendingQuestName) - 1);
    _pendingAlarmIdx = alarmIdx;
    _nextStateType = StateType::Notification;
    _stateChangeRequested = true;
}

void App::logNotAnswered(const char* questName) {

    char ts[20];
    formatNow(ts, sizeof(ts));

    _data.saveResponse(
        ts,
        _config.subjectId(),
        questName,
        "not_answered",
        "not_answered",
        "not_answered",
        -1,
        0
    );
    Serial.printf("Fragebogen %s als not_answered geloggt\n", questName);
}

void App::enterNextState() {
    if (_currentState) {
        _currentState->exit();
        delete _currentState;
        _currentState = nullptr;
    }

    switch (_nextStateType) {
        case StateType::Home:
            _currentState = new HomeState(this);
            break;

        case StateType::Settings:
            _currentState = new SettingsState(this);
            break;

        case StateType::LockScreen:
            if (strcmp(_nextStateContext, "delete") == 0)
                _currentState = new LockScreenState(this, LockScreenState::Action::UnlockDelete);
            else
                _currentState = new LockScreenState(this, LockScreenState::Action::UnlockSettings);
            break;

        case StateType::Questionnaire: {
            JsonObjectConst q;
            if (_pendingQuestScheduled)
                q = _config.getQuestionnaireByName(_pendingQuestName);
            else
                q = _config.getEventQuestionnaire(_pendingQuestIdx);

            if (q.isNull()) {
                _currentState = new DoneState(this, "Fragebogen nicht gefunden!");
            } else {

                _currentState = new QuestionnaireState(this, q, !_pendingQuestScheduled);
            }
            break;
        }

        case StateType::Notification:
            _currentState = new NotificationState(this, _pendingQuestName, _pendingAlarmIdx);
            break;

        case StateType::Done:
            _currentState = new DoneState(this, _nextStateContext);
            break;

        default:
            _currentState = new HomeState(this);
            break;
    }

    if (_currentState) {
        _currentState->enter();
        _power.resetInactivityTimer();
    }
}

void App::initHardware() {

    instance.begin(NO_INIT_FATFS | NO_HW_GPS | NO_HW_LORA | NO_HW_MIC | NO_SCAN_I2C_DEV);

    instance.powerControl(POWER_GPS, false);

    instance.sensor.disableTiltIRQ();
    instance.sensor.disableWakeupIRQ();
    instance.sensor.disableAnyNoMotionIRQ();
    instance.sensor.disableActivityIRQ();

    instance.setBrightness(_brightness);
    beginLvglHelper(instance);

    instance.onEvent(onPmuEvent, POWER_EVENT, this);
}

void App::onPmuEvent(DeviceEvent_t event, void* params, void* userData) {
    App* self = static_cast<App*>(userData);
    PMUEventType_t type = instance.getPMUEventType(params);
    self->handlePmuButton(type);
}

void App::handlePmuButton(PMUEventType_t type) {
    if (type == PMU_EVENT_KEY_CLICKED) {

        if (_currentState && _currentState->type() == StateType::Notification) {
            requestState(StateType::Home);
            return;
        }

        if (_power.isDisplayOn()) {
            _power.sleepDisplay();
        } else {
            _power.wakeDisplay();
        }
    } else if (type == PMU_EVENT_KEY_LONG_PRESSED) {

        Serial.println("Long-Press: Shutdown");
        Serial.flush();
        instance.setBrightness(0);
        delay(100);
        instance.pmu.shutdown();
    } else if (type == PMU_EVENT_USBC_INSERT) {

        Serial.println("USB-C Insert");
        _power.resetInactivityTimer();
        if (!_power.isDisplayOn()) _power.wakeDisplay();
        invalidateBattAndRedraw();
    } else if (type == PMU_EVENT_USBC_REMOVE) {

        Serial.println("USB-C Remove");
        invalidateBattAndRedraw();
    } else if (type == PMU_EVENT_CHARGE_STARTED || type == PMU_EVENT_CHARGE_FINISH) {

        invalidateBattAndRedraw();
    }
}

void App::invalidateBattAndRedraw() {
    _battLastRead = 0;
    if (_currentState) _currentState->update();
    _lastDisplayUpdate = millis();
}

void App::formatNow(char* buf, size_t len) {
    struct tm t;
    instance.rtc.getDateTime(&t);
    snprintf(buf, len, "%02d.%02d.%04d %02d:%02d:%02d",
             t.tm_mday, t.tm_mon + 1, t.tm_year + 1900,
             t.tm_hour, t.tm_min, t.tm_sec);
}

uint8_t App::batteryPercent() {

    if (_battLastRead == 0 || millis() - _battLastRead >= 30000) {
        int bp = instance.pmu.getBatteryPercent();
        _battCached = (bp < 0) ? 0 : (bp > 100 ? 100 : (uint8_t)bp);
        _battLastRead = millis();
    }
    return _battCached;
}

bool App::isCharging() {
    return instance.pmu.isCharging();
}

void App::vibrate(uint8_t effect) {
    instance.drv.setWaveform(0, effect);
    instance.drv.setWaveform(1, 0);
    instance.drv.run();
}

void App::setBrightness(uint8_t v) {
    if (v < 8) v = 8;
    _brightness = v;
    instance.setBrightness(v);
    Preferences prefs;
    prefs.begin("esm", false);
    prefs.putUChar("bright", v);
    prefs.end();
}

void App::setUsbMode(bool on) {
    _usbMode = on;
    if (on) {

        _lastSerialActivity = millis();
        _power.resetInactivityTimer();
        Serial.println("USB-Modus: AN (Light-Sleep deaktiviert)");
    } else {
        Serial.println("USB-Modus: AUS");
    }

    if (_currentState) {
        _currentState->update();
        _lastDisplayUpdate = millis();
    }
}

void App::toggleTheme() {
    g_darkMode = !g_darkMode;
    Preferences prefs;
    prefs.begin("esm", false);
    prefs.putBool("dark", g_darkMode);
    prefs.end();
    stylesReinit();

    requestState(StateType::Settings);
}

void App::cycleWallpaper() {
    g_wallpaper = (g_wallpaper + 1) % WALLPAPER_COUNT;
    Preferences prefs;
    prefs.begin("esm", false);
    prefs.putUChar("wp", g_wallpaper);
    prefs.end();
    requestState(StateType::Settings);
}

void App::checkScheduledAlarms() {
    if (!_config.isLoaded()) return;

    if (!_currentState || _currentState->type() != StateType::Home) return;

    struct tm t;
    instance.rtc.getDateTime(&t);

    int year = t.tm_year + 1900;
    if (year < 2025) return;

    bool freshTrigger = false;
    int8_t idx = _schedule.checkAlarms(t.tm_mday, t.tm_hour, t.tm_min, &freshTrigger);

    char expiredName[MAX_NAME_LEN];
    if (_schedule.consumeAutoExpired(expiredName, sizeof(expiredName))) {
        logNotAnswered(expiredName);
    }

    if (idx >= 0 && freshTrigger) {
        vibrate(47);
        if (!_power.isDisplayOn()) {
            _power.wakeDisplay();
        }
        showNotification(_schedule.alarmQuestName(idx), idx);
    }

}

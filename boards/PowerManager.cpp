// PowerManager — Display + Light-Sleep Verwaltung
#include "services/PowerManager.h"
#include "App.h"
#include <LilyGoLib.h>

void PowerManager::sleepDisplay() {
    if (!_displayOn) return;
    _displayOn = false;
    instance.setBrightness(0);
}

void PowerManager::wakeDisplay() {
    if (_displayOn) return;
    _displayOn = true;
    _sleeping = false;
    instance.setBrightness(_app->config().brightness());
    _lastActivity = millis();
}

esp_sleep_wakeup_cause_t PowerManager::enterLightSleep() {

    if (Serial) {
        _sleeping = false;
        return ESP_SLEEP_WAKEUP_UNDEFINED;
    }

    _displayOn = false;
    _sleeping = true;

    struct tm t;
    instance.rtc.getDateTime(&t);
    uint32_t secsToAlarm = _app->schedule().secondsToNextAlarm(t.tm_hour, t.tm_min);

    if (secsToAlarm < 30)   secsToAlarm = 30;
    if (secsToAlarm > 3600) secsToAlarm = 3600;

    Serial.printf("Light-Sleep: %lu Sek bis naechster Alarm\n", secsToAlarm);
    Serial.flush();

    esp_sleep_enable_timer_wakeup((uint64_t)secsToAlarm * 1000000ULL);

    instance.lightSleep(
        (WakeupSource_t)(WAKEUP_SRC_POWER_KEY | WAKEUP_SRC_TOUCH_PANEL)
    );

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    _sleeping = false;
    _lastActivity = millis();

    Serial.printf("Wakeup: Ursache=%d\n", (int)cause);

    return cause;
}

void PowerManager::resetInactivityTimer() {
    _lastActivity = millis();
}

bool PowerManager::isInactivityTimeout() const {
    uint16_t timeout = _app->config().isLoaded()
        ? _app->config().displayTimeoutMs()
        : DEFAULT_DISPLAY_TIMEOUT_MS;
    return (millis() - _lastActivity >= timeout);
}

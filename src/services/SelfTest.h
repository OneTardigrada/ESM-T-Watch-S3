#pragma once

#include <Arduino.h>

class App;

class SelfTest {
public:
    explicit SelfTest(App* app) : _app(app) {}

    void runAll();

private:
    App* _app;
    int  _pass = 0;
    int  _fail = 0;

    void report(const char* name, bool ok, const char* detail = nullptr);

    void testCpuFrequency();
    void testNotifTimeoutConst();
    void testNightDayBoundary();
    void testCryptoRoundtrip();
    void testCryptoIvUnique();
    void testCryptoOversize();
    void testCryptoHeapStable();
    void testFormatNowBuffer();
    void testInvalidateBattHelper();
};

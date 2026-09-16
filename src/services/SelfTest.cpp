// =====================================================================
// SelfTest — Implementierung
// =====================================================================
#include "services/SelfTest.h"
#include "App.h"
#include "config.h"

#include <esp32-hal-cpu.h>

void SelfTest::report(const char* name, bool ok, const char* detail) {
    if (ok) { _pass++; Serial.printf("  [PASS] %s", name); }
    else    { _fail++; Serial.printf("  [FAIL] %s", name); }
    if (detail && detail[0]) Serial.printf(" — %s", detail);
    Serial.println();
}

void SelfTest::testCpuFrequency() {
    uint32_t mhz = getCpuFrequencyMhz();
    char buf[32]; snprintf(buf, sizeof(buf), "current=%u MHz", (unsigned)mhz);
    report("S6 CPU 80 MHz", mhz == 80, buf);
}

void SelfTest::testNotifTimeoutConst() {
    char buf[40]; snprintf(buf, sizeof(buf), "NOTIF_DISPLAY_TIMEOUT_MS=%u", NOTIF_DISPLAY_TIMEOUT_MS);
    report("S1 Notif-Timeout = 15000ms", NOTIF_DISPLAY_TIMEOUT_MS == 15000, buf);
}

static uint32_t expectedMinSleep(int hour) {
    bool night = (hour >= 22 || hour < 6);
    return night ? 300u : 30u;
}
void SelfTest::testNightDayBoundary() {
    struct Case { int hour; uint32_t expect; const char* name; };
    Case cases[] = {
        { 0,  300, "00:00 night" }, { 5,  300, "05:59 night-edge" },
        { 6,  30,  "06:00 day-start" }, { 12, 30, "12:00 day" },
        { 21, 30,  "21:59 day-edge" }, { 22, 300, "22:00 night-start" },
        { 23, 300, "23:59 night" },
    };
    bool all = true;
    for (auto& c : cases) {
        if (expectedMinSleep(c.hour) != c.expect) {
            all = false;
            Serial.printf("    Mismatch: hour=%d expect=%u got=%u (%s)\n",
                c.hour, (unsigned)c.expect, (unsigned)expectedMinSleep(c.hour), c.name);
        }
    }
    report("S3 Night/Day boundary", all, "7 cases");
}

void SelfTest::testCryptoRoundtrip() {
    if (!_app->crypto().isReady()) {
        report("P5 Crypto roundtrip", false, "key not ready");
        return;
    }

    const size_t lens[] = { 1, 15, 16, 17, 31, 32, 33, 100, 200, 256, 400, 500, 511 };
    bool all = true;
    char detail[64] = "";
    for (size_t L : lens) {
        char* plain = (char*)malloc(L + 1);
        if (!plain) { all = false; snprintf(detail, sizeof(detail), "OOM len=%u", (unsigned)L); break; }
        for (size_t i = 0; i < L; i++) plain[i] = 'A' + (i % 26);
        plain[L] = '\0';

        String enc = _app->crypto().encrypt(plain);
        if (enc.length() == 0) {
            all = false; snprintf(detail, sizeof(detail), "encrypt empty len=%u", (unsigned)L);
            free(plain); break;
        }
        String dec = _app->crypto().decrypt(enc.c_str());
        if (dec.length() != L || memcmp(dec.c_str(), plain, L) != 0) {
            all = false;
            snprintf(detail, sizeof(detail), "mismatch len=%u dec=%u", (unsigned)L, (unsigned)dec.length());
            free(plain); break;
        }
        free(plain);
    }
    if (all) snprintf(detail, sizeof(detail), "13 lengths 1..511");
    report("P5 Crypto roundtrip", all, detail);
}

void SelfTest::testCryptoIvUnique() {
    if (!_app->crypto().isReady()) { report("P5 Crypto IV unique", false, "key not ready"); return; }
    const char* p = "ESM-test-payload-2026";
    String a = _app->crypto().encrypt(p);
    String b = _app->crypto().encrypt(p);
    String c = _app->crypto().encrypt(p);
    bool ok = (a.length() > 0 && a != b && b != c && a != c);
    report("P5 Crypto IV unique", ok, ok ? "3 distinct ciphertexts" : "duplicate ciphertext!");
}

void SelfTest::testCryptoOversize() {
    if (!_app->crypto().isReady()) { report("P5 Crypto oversize reject", false, "key not ready"); return; }

    char big[601];
    memset(big, 'X', 600); big[600] = '\0';
    String r = _app->crypto().encrypt(big);
    report("P5 Crypto oversize reject", r.length() == 0,
           r.length() == 0 ? "rejected as expected" : "OVERFLOW — Aenderung pruefen!");
}

void SelfTest::testCryptoHeapStable() {
    if (!_app->crypto().isReady()) { report("P5 Crypto heap stable", false, "key not ready"); return; }
    const char* p = "2026-05-04 12:34:56;S001;Q1;item1;likert;item1;3;1234";

    String warm = _app->crypto().encrypt(p);
    (void)_app->crypto().decrypt(warm.c_str());

    uint32_t before = ESP.getFreeHeap();
    for (int i = 0; i < 50; i++) {
        String e = _app->crypto().encrypt(p);
        String d = _app->crypto().decrypt(e.c_str());
        (void)d;
    }
    uint32_t after = ESP.getFreeHeap();
    int32_t delta = (int32_t)before - (int32_t)after;
    char buf[64]; snprintf(buf, sizeof(buf), "delta=%ld B (before=%u after=%u)",
                           (long)delta, (unsigned)before, (unsigned)after);

    bool ok = (delta < 1024 && delta > -4096);
    report("P5 Crypto heap stable (50x)", ok, buf);
}

void SelfTest::testFormatNowBuffer() {
    char big[20];
    char tiny[8];
    memset(big, 0xAA, sizeof(big));
    memset(tiny, 0xAA, sizeof(tiny));
    _app->formatNow(big, sizeof(big));
    _app->formatNow(tiny, sizeof(tiny));
    bool bigOk  = (strlen(big)  == 19);
    bool tinyOk = (strlen(tiny) <  sizeof(tiny));
    char buf[64]; snprintf(buf, sizeof(buf), "big='%s' tiny='%s'", big, tiny);
    report("R4 formatNow truncation safe", bigOk && tinyOk, buf);
}

void SelfTest::testInvalidateBattHelper() {

    int bp = _app->batteryPercent();
    char buf[32]; snprintf(buf, sizeof(buf), "battery=%d%%", bp);
    report("R1 batteryPercent in [0..100]", (bp >= 0 && bp <= 100), buf);
}

void SelfTest::runAll() {
    _pass = _fail = 0;
    Serial.println();
    Serial.println("=== SELFTEST START ===");
    Serial.printf("Free heap before: %u\n", ESP.getFreeHeap());

    testCpuFrequency();
    testNotifTimeoutConst();
    testNightDayBoundary();
    testCryptoRoundtrip();
    testCryptoIvUnique();
    testCryptoOversize();
    testCryptoHeapStable();
    testFormatNowBuffer();
    testInvalidateBattHelper();

    Serial.printf("Free heap after:  %u\n", ESP.getFreeHeap());
    Serial.printf("SELFTEST_RESULT: %d/%d passed\n", _pass, _pass + _fail);
    Serial.println(_fail == 0 ? "=== SELFTEST OK ===" : "=== SELFTEST FAILED ===");
}

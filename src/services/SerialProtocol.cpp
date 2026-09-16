// SerialProtocol — Authentifiziertes Command-Interface
#include "services/SerialProtocol.h"
#include "services/SelfTest.h"
#include "App.h"
#include <LittleFS.h>

void SerialProtocol::process() {
    if (!Serial.available()) return;

    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    if (_authenticated && (millis() - _authTime > AUTH_SESSION_TIMEOUT_MS)) {
        resetSession();
        Serial.println("SESSION_EXPIRED");
    }

    handleCommand(input);
}

void SerialProtocol::handleCommand(const String& cmd) {
    if (cmd.startsWith("AUTH:")) {
        cmdAuth(cmd.substring(5));
    } else if (cmd == "EXPORT") {
        cmdExport();
    } else if (cmd.startsWith("CONFIG_START")) {
        cmdConfig();
    } else if (cmd == "CONFIG_GET") {
        cmdConfigGet();
    } else if (cmd == "DELETE") {
        cmdDelete();
    } else if (cmd == "STATUS") {
        cmdStatus();
    } else if (cmd == "info") {
        cmdInfo();
    } else if (cmd == "help") {
        cmdHelp();
    } else if (cmd == "SELFTEST") {
        cmdSelfTest();
    } else {

        if (cmd == "export") {
            Serial.println("FEHLER: Bitte AUTH:<pin> senden. 'help' fuer Befehle.");
        } else {
            Serial.println("UNKNOWN_COMMAND");
        }
    }
}

void SerialProtocol::cmdAuth(const String& token) {
    const char* correctPin = _app->config().pin();
    if (token == correctPin) {
        _authenticated = true;
        _authTime = millis();
        Serial.println("AUTH_OK");
    } else {
        _authenticated = false;
        Serial.println("AUTH_FAIL");
    }
}

bool SerialProtocol::checkAuth() {
    if (!_authenticated) {
        Serial.println("AUTH_REQUIRED");
        return false;
    }
    if (millis() - _authTime > AUTH_SESSION_TIMEOUT_MS) {
        resetSession();
        Serial.println("SESSION_EXPIRED");
        return false;
    }
    _authTime = millis();
    return true;
}

void SerialProtocol::cmdExport() {
    if (!checkAuth()) return;
    _app->data().exportSerial();
}

void SerialProtocol::cmdConfig() {
    if (!checkAuth()) return;

    Serial.println("CONFIG_READY");

    String json;
    json.reserve(8192);
    Serial.setTimeout(2000);
    unsigned long timeout = millis() + 30000;

    while (millis() < timeout) {
        if (Serial.available()) {
            String line = Serial.readStringUntil('\n');
            line.trim();
            if (line == "CONFIG_END") break;
            if (line.length() > 0) json += line;
            timeout = millis() + 30000;
        }
        yield();
    }
    Serial.setTimeout(1000);

    if (json.length() == 0) {
        Serial.println("CONFIG_ERROR:Keine Daten empfangen");
        return;
    }

    Serial.printf("CONFIG_RECV:%u\n", (unsigned)json.length());

    if (_app->config().parseAndStore(json)) {

        _app->crypto().deriveKey(_app->config().pin());

        _app->schedule().loadFromConfig();
        Serial.println("CONFIG_OK");
    } else {
        Serial.println("CONFIG_ERROR:Parse-Fehler");
    }
}

void SerialProtocol::cmdConfigGet() {
    if (!checkAuth()) return;

    File f = LittleFS.open(CONFIG_FILE, "r");
    if (!f) {
        Serial.println("CONFIG_GET_ERROR:Keine Config vorhanden");
        return;
    }

    Serial.println("CONFIG_GET_START");
    while (f.available()) {
        Serial.write(f.read());
    }
    Serial.println();
    Serial.println("CONFIG_GET_END");
    f.close();
}

void SerialProtocol::cmdDelete() {
    if (!checkAuth()) return;
    _app->data().deleteAll();
    Serial.println("DELETE_OK");
}

void SerialProtocol::cmdStatus() {

    Serial.printf("STATUS:%s;%d;%d;%lu\n",
                  _app->config().subjectId(),
                  _app->data().rowCount(),
                  _app->batteryPercent(),
                  millis() / 1000);
}

void SerialProtocol::cmdInfo() {
    Serial.println("=== ESM INFO ===");
    Serial.printf("Subject: %s\n", _app->config().subjectId());
    Serial.printf("Studie: %s\n", _app->config().studyName());
    Serial.printf("Antworten: %d\n", _app->data().rowCount());
    Serial.printf("Config: %s\n", _app->config().isLoaded() ? "JA" : "NEIN");
    Serial.printf("Crypto: %s\n", _app->crypto().isReady() ? "JA" : "NEIN");
    Serial.printf("Alarme: %d\n", _app->schedule().alarmCount());
    Serial.printf("Akku: %d%%\n", _app->batteryPercent());
    Serial.printf("Heap: %u\n", ESP.getFreeHeap());
    Serial.printf("PSRAM: %u\n", ESP.getFreePsram());
    Serial.println("=== ENDE ===");
}

void SerialProtocol::cmdHelp() {
    Serial.println("=== ESM Serial-Protokoll ===");
    Serial.println("AUTH:<pin>     - Authentifizieren");
    Serial.println("EXPORT         - Daten exportieren (auth)");
    Serial.println("CONFIG_START   - Config senden (auth)");
    Serial.println("CONFIG_GET     - Aktuelle Config holen (auth)");
    Serial.println("DELETE         - Daten loeschen (auth)");
    Serial.println("STATUS         - System-Status");
    Serial.println("info           - Detaillierte Info");
    Serial.println("help           - Diese Hilfe");
    Serial.println("SELFTEST       - Phase-4 Selbsttests ausfuehren");
    Serial.println("============================");
}

void SerialProtocol::resetSession() {
    _authenticated = false;
    _authTime = 0;
}

void SerialProtocol::cmdSelfTest() {

    SelfTest st(_app);
    st.runAll();
}

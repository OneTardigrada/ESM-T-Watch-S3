// DataManager — Verschluesselte CSV-Speicherung in LittleFS
#include "services/DataManager.h"
#include "services/CryptoManager.h"
#include <LittleFS.h>

static const char* CSV_HEADER = "timestamp;subject_id;questionnaire;item_name;type;prompt;response;duration_ms";

bool DataManager::begin() {
    return LittleFS.begin(true);
}

bool DataManager::saveResponse(const char* timestamp,
                               const char* subjectId,
                               const char* questName,
                               const char* itemName,
                               const char* itemType,
                               const char* prompt,
                               int16_t response,
                               unsigned long durationMs) {
    bool exists = LittleFS.exists(DATA_FILE);
    File f = LittleFS.open(DATA_FILE, "a");
    if (!f) return false;

    if (!exists) {

        f.println(CSV_HEADER);
    }

    char line[512];
    snprintf(line, sizeof(line), "%s;%s;%s;%s;%s;%s;%d;%lu",
             timestamp, subjectId, questName, itemName,
             itemType, prompt, response, durationMs);

    if (_crypto.isReady()) {
        String encrypted = _crypto.encrypt(line);
        if (encrypted.length() > 0) {
            f.println(encrypted);
        } else {
            f.println(line);
        }
    } else {
        f.println(line);
    }

    f.close();
    return true;
}

void DataManager::exportSerial() {
    Serial.println("=== DATEN-EXPORT START ===");

    if (!LittleFS.exists(DATA_FILE)) {
        Serial.println("Keine Daten vorhanden");
        Serial.println("=== DATEN-EXPORT ENDE ===");
        return;
    }

    File f = LittleFS.open(DATA_FILE, "r");
    if (!f) {
        Serial.println("FEHLER: Datei nicht lesbar");
        Serial.println("=== DATEN-EXPORT ENDE ===");
        return;
    }

    bool firstLine = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        if (firstLine) {

            Serial.println(line);
            firstLine = false;
            continue;
        }

        if (_crypto.isReady()) {
            String decrypted = _crypto.decrypt(line.c_str());
            Serial.println(decrypted.length() > 0 ? decrypted : line);
        } else {
            Serial.println(line);
        }
    }
    f.close();
    Serial.println("=== DATEN-EXPORT ENDE ===");
}

bool DataManager::deleteAll() {
    if (LittleFS.exists(DATA_FILE)) {
        return LittleFS.remove(DATA_FILE);
    }
    return true;
}

uint16_t DataManager::rowCount() {
    if (!LittleFS.exists(DATA_FILE)) return 0;
    File f = LittleFS.open(DATA_FILE, "r");
    if (!f) return 0;

    uint16_t count = 0;
    while (f.available()) {
        f.readStringUntil('\n');
        count++;
    }
    f.close();
    return (count > 0) ? count - 1 : 0;
}

uint16_t DataManager::countCompleted(const char* questName, const char* lastItemName) {
    if (!LittleFS.exists(DATA_FILE)) return 0;
    File f = LittleFS.open(DATA_FILE, "r");
    if (!f) return 0;

    uint16_t count = 0;
    bool firstLine = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (firstLine) { firstLine = false; continue; }
        if (line.length() == 0) continue;

        String decoded = line;
        if (_crypto.isReady()) {
            String d = _crypto.decrypt(line.c_str());
            if (d.length() > 0) decoded = d;
        }

        if (decoded.indexOf(questName) >= 0 && decoded.indexOf(lastItemName) >= 0) {
            count++;
        }
    }
    f.close();
    return count;
}

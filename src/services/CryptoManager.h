#pragma once

#include <Arduino.h>
#include "config.h"

class CryptoManager {
public:

    void deriveKey(const char* pin);

    bool isReady() const { return _ready; }

    String encrypt(const char* plaintext);

    String decrypt(const char* encoded);

private:
    uint8_t _key[AES_KEY_BYTES] = {};
    bool    _ready = false;

    void generateIV(uint8_t* iv);
};

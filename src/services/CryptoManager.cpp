// CryptoManager — AES-256-CBC mit mbedtls (ESP32-native)
#include "services/CryptoManager.h"
#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"
#include "mbedtls/base64.h"
#include "esp_random.h"
#include "esp_mac.h"
#include <cstring>

void CryptoManager::deriveKey(const char* pin) {
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, (const uint8_t*)pin, strlen(pin));
    mbedtls_sha256_update(&ctx, mac, sizeof(mac));
    mbedtls_sha256_finish(&ctx, _key);
    mbedtls_sha256_free(&ctx);

    _ready = true;
}

void CryptoManager::generateIV(uint8_t* iv) {
    for (int i = 0; i < AES_IV_BYTES; i += 4) {
        uint32_t r = esp_random();
        memcpy(iv + i, &r, (i + 4 <= AES_IV_BYTES) ? 4 : AES_IV_BYTES - i);
    }
}

String CryptoManager::encrypt(const char* plaintext) {
    if (!_ready) return "";
    constexpr size_t MAX_PLAIN  = 512;
    constexpr size_t B64_BUF    = 768;
    size_t ptLen = strlen(plaintext);

    size_t padLen = AES_BLOCK_SIZE - (ptLen % AES_BLOCK_SIZE);
    size_t ctLen  = ptLen + padLen;
    if (ctLen > MAX_PLAIN) {
        Serial.printf("Crypto: plaintext zu lang (%u > %u)\n", (unsigned)ctLen, (unsigned)MAX_PLAIN);
        return "";
    }

    uint8_t padded[MAX_PLAIN];
    memcpy(padded, plaintext, ptLen);
    memset(padded + ptLen, (uint8_t)padLen, padLen);

    uint8_t iv[AES_IV_BYTES];
    generateIV(iv);
    uint8_t ivCopy[AES_IV_BYTES];
    memcpy(ivCopy, iv, AES_IV_BYTES);

    uint8_t ct[MAX_PLAIN];
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, _key, AES_KEY_BYTES * 8);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, ctLen, ivCopy, padded, ct);
    mbedtls_aes_free(&aes);

    char b64Iv[32];
    char b64Ct[B64_BUF];
    size_t b64IvLen = 0, b64CtLen = 0;
    if (mbedtls_base64_encode((uint8_t*)b64Iv, sizeof(b64Iv) - 1, &b64IvLen, iv, AES_IV_BYTES) != 0) return "";
    if (mbedtls_base64_encode((uint8_t*)b64Ct, sizeof(b64Ct) - 1, &b64CtLen, ct, ctLen) != 0) return "";
    b64Iv[b64IvLen] = '\0';
    b64Ct[b64CtLen] = '\0';

    String result;
    result.reserve(b64IvLen + 1 + b64CtLen);
    result += b64Iv;
    result += ':';
    result += b64Ct;
    return result;
}

String CryptoManager::decrypt(const char* encoded) {
    if (!_ready) return "";

    String input(encoded);
    int sep = input.indexOf(':');
    if (sep < 0) return "";

    String b64Iv = input.substring(0, sep);
    String b64Ct = input.substring(sep + 1);

    uint8_t iv[AES_IV_BYTES];
    size_t ivLen = AES_IV_BYTES;
    if (mbedtls_base64_decode(iv, AES_IV_BYTES, &ivLen,
                               (const uint8_t*)b64Iv.c_str(), b64Iv.length()) != 0)
        return "";

    size_t ctMaxLen = b64Ct.length();
    uint8_t* ct = (uint8_t*)malloc(ctMaxLen);
    if (!ct) return "";
    size_t ctLen = 0;
    if (mbedtls_base64_decode(ct, ctMaxLen, &ctLen,
                               (const uint8_t*)b64Ct.c_str(), b64Ct.length()) != 0) {
        free(ct);
        return "";
    }

    uint8_t* pt = (uint8_t*)malloc(ctLen + 1);
    if (!pt) { free(ct); return ""; }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, _key, AES_KEY_BYTES * 8);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ctLen, iv, ct, pt);
    mbedtls_aes_free(&aes);
    free(ct);

    if (ctLen == 0) { free(pt); return ""; }
    uint8_t padVal = pt[ctLen - 1];
    if (padVal == 0 || padVal > AES_BLOCK_SIZE) { free(pt); return ""; }
    size_t ptLen = ctLen - padVal;
    pt[ptLen] = '\0';

    String result((char*)pt);
    free(pt);
    return result;
}

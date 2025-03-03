#include "../libtuya.hpp"

//
//#define AES128 1
//#define AES_BLOCKLEN 16
//#include <mbedtls/gcm.h>

//
namespace tc {
/*
    // ESP32-S3 or CardPuter won't support native AES GCM, but tuya protocol 3.5 requires it
    uint8_t* decryptDataGCM(uint8_t* key, uint8_t* data, size_t& length,  uint8_t* output) {
        if (!output) { output = data + 12; };

        //
        mbedtls_gcm_context aes;
        mbedtls_gcm_init(&aes);
        mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, (const unsigned char*) key, 128);
        mbedtls_gcm_crypt_and_tag(&aes, MBEDTLS_GCM_DECRYPT,
            length, // length of payload only
            (const unsigned char*)data, 12, // prepared IV
            NULL, 0,
            (unsigned char const*)data+12, // payload stream
            (unsigned char*) output, // output (if present)
            16, output + length);
        mbedtls_gcm_free(&aes);
        return output;
    }

    // ESP32-S3 or CardPuter won't support native AES GCM, but tuya protocol 3.5 requires it
    uint8_t* encryptDataGCM(uint8_t* key, uint8_t* data, size_t& length,  uint8_t* output) {
        if (!output) { output = data + 12; };

        //
        mbedtls_gcm_context aes;
        mbedtls_gcm_init(&aes);
        mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES , (const unsigned char*) key, 128);
        mbedtls_gcm_crypt_and_tag(&aes, MBEDTLS_GCM_ENCRYPT,
            length, // length of payload only
            (const unsigned char*)data, 12, // prepared IV
            NULL, 0,
            (unsigned char const*)data+12, // payload stream
            (unsigned char*) output, // output (if present)
            16, output + length);
        mbedtls_gcm_free(&aes);
        return output;
    }
*/
}

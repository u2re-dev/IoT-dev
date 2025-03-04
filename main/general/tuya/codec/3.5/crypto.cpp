#include "../libtuya.hpp"

//
#define AES128 1
#define AES_BLOCKLEN 16
#include <mbedtls/gcm.h>

//
namespace tc {
    // ESP32-S3 or CardPuter won't support native AES GCM, but tuya protocol 3.5 requires it
    bytespan_t decryptDataGCM(block_t const& key, bytespan_t& iv_payload_tag) {
        auto output = bytespan_t(iv_payload_tag->data() + 12, iv_payload_tag->size() - 12 - 16);
        mbedtls_gcm_context aes;
        mbedtls_gcm_init(&aes);
        mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&key), 128);
        mbedtls_gcm_crypt_and_tag(&aes, MBEDTLS_GCM_DECRYPT,
            output->size(), // length of payload only
            iv_payload_tag->data(), 12, // prepared IV
            NULL, 0,
            output->data(), // payload stream
            output->data(), // output (if present)
            16, output->data()+(output->size()+12)
        );
        mbedtls_gcm_free(&aes);
        return output;
    }

    // ESP32-S3 or CardPuter won't support native AES GCM, but tuya protocol 3.5 requires it
    bytespan_t encryptDataGCM(block_t const& key, bytespan_t& iv_payload_tag) {
        auto output = bytespan_t(iv_payload_tag->data() + 12, iv_payload_tag->size() - 12 - 16);
        mbedtls_gcm_context aes;
        mbedtls_gcm_init(&aes);
        mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&key), 128);
        mbedtls_gcm_crypt_and_tag(&aes, MBEDTLS_GCM_ENCRYPT,
            output->size(), // length of payload only
            iv_payload_tag->data(), 12, // prepared IV
            NULL, 0,
            output->data(), // payload stream
            output->data(), // output (if present)
            16, output->data()+(output->size()+12)
        );
        mbedtls_gcm_free(&aes);
        return output;
    }
}

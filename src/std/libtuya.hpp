//
#include <std/std.hpp>

//
#include "../crypto/soc.hpp"
#include "utils.hpp"
#define AES128 1
#define AES_BLOCKLEN 16

//
namespace tc {

    //
    void decryptDataCBC(uint8_t* key,  uint8_t* iv,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);
        esp_aes_crypt_cbc(&ctx, ESP_AES_DECRYPT, length, iv, data, output);
    }

    //
    void encryptDataCBC(uint8_t* key,  uint8_t* iv,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);
        esp_aes_crypt_cbc(&ctx, ESP_AES_ENCRYPT, length, iv, data, output);
    }

    //
    struct TuyaCmd {
        uint32_t SEQ_NO = 0;
        uint32_t CMD_ID = 0; 
        uint8_t* HMAC = nullptr;
    };

    //
    void encryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output, const bool usePadding = true) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        // prepare output to encrypt
        memcpy(output, data, length);

        // add post-padding
        const auto pad = ((length + 16 /*- 1*/) >> 4) << 4;
        if (usePadding) {
            for (uint I=0;I<pad;I++) {
                output[length+I] = pad;
            }
        }

        //
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, output+I, output+I);
        }

        // add padding value
        if (usePadding) { length += pad; };
    }

    //
    void decryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        //
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, data+I, output+I);
        }

        // re-correction of length (if possible)
        const auto pad = data[length-1];
        if (pad <= 16 && pad > 0 && length > 16) { length -= pad; };
    }

    //
    uint32_t computePayloadSize(uint32_t payloadLen, bool hmac) {
        return payloadLen + (hmac ? 32 : 4) + 4;
    }

    //
    uint32_t computeCodeSize(uint32_t payloadLen, bool hmac) {
        return 16 + computePayloadSize(payloadLen, hmac);
    }

    //
    std::array<uint32_t, 2> prepareJSON(uint8_t*data, size_t& length, char const* protocolVersion, uint8_t* output) {
        // protocol 3.4 - encrypted with header
        const auto encLen = ((length + 15 + 16) >> 4) << 4;
        const auto encOffset = 0;

        // protocol 3.3 - encrypted data only
        //const auto encLen = ((length + 16) >> 4) << 4;
        //const auto encOffset = 15;

        //
        if (!output) output = (uint8_t*)calloc(1, encLen + encOffset);
        memcpy(output, protocolVersion, 3);
        // [4...15) i.e. 12 bytes - something
        memcpy(output + 15, data, length);

        // return encryptable zone
        return std::array<uint32_t, 2>{encOffset, encLen};
    }

    // HMAC i.e. hmac_key
    uint8_t* encodeTuyaCode(uint8_t* encrypted_data, size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr) {
        // write header
        *(uint32_t*)(output+0) = 0x000055AA;

        // encode as big-endian
        *(uint32_t*)(output+4)  = bswap32(cmdDesc.SEQ_NO);
        *(uint32_t*)(output+8)  = bswap32(cmdDesc.CMD_ID);
        *(uint32_t*)(output+12) = bswap32(computePayloadSize(length, cmdDesc.HMAC ? true : false));

        //
        const uint32_t header_len = 16; size_t key_len = 16;
        const auto payload = output + header_len;
        memcpy(payload, encrypted_data, length);
        if (cmdDesc.HMAC) {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, (const unsigned char *) cmdDesc.HMAC, key_len);
            mbedtls_md_hmac_update(&ctx, (const unsigned char *) output, length + header_len); // header + payload
            mbedtls_md_hmac_finish(&ctx, payload + length); // write after payload
            mbedtls_md_free(&ctx);
        } else {
            *(uint32_t*)(payload + length) = crc32_be(0, output, length + header_len);
        }

        // write suffix
        *(uint32_t*)(output+0) = 0x0000AA55;

        //
        return output;
    }


    // for protocol 3.4, remote_nonce is encrypted
    uint8_t* encode_remote_hmac(uint8_t* original_key, uint8_t* remote_nonce, size_t length = 0,   uint8_t* remote_hmac = nullptr) {
        size_t key_len = 16;
        decryptDataECB(original_key, remote_nonce, key_len, remote_nonce);

        //
        size_t hmac_length = 48;
        remote_hmac = remote_hmac ? remote_hmac : (uint8_t*)calloc(1, hmac_length);

        //
#ifdef CONFIG_IDF_TARGET_ESP32S3
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
        mbedtls_md_hmac_starts(&ctx, (const unsigned char *) original_key, key_len);
        mbedtls_md_hmac_update(&ctx, (const unsigned char *) remote_nonce, key_len);
        mbedtls_md_hmac_finish(&ctx, remote_hmac);
        mbedtls_md_free(&ctx);
#else
        sf_hmac_sha256(original_key, key_len, remote_nonce, key_len, remote_hmac, hmac_length);
#endif

        encryptDataECB(original_key, remote_hmac, hmac_length, remote_hmac, false);

        // needs to send these data
        return remote_hmac;
    }

    //
    uint8_t* encode_hmac_key(uint8_t* original_key, uint8_t* remote_nonce,  uint8_t* hmac_key = nullptr) {
        size_t key_len = 16;
        const uint8_t* local_nonce = "0123456789abcdef";

        //
        uint32_t* loc = hmac_key ? (uint32_t*)hmac_key : (uint32_t*)calloc(1, key_len);
        const uint32_t* loc_n = (uint32_t*)local_nonce;
        const uint32_t* rem_n = (uint32_t*)remote_nonce;
        for (uint8_t I=0;I<4;I++) { loc[I] = loc_n[I]^rem_n[I]; }

        //
        uint8_t* local_key = (uint8_t*)loc; // encode without padding
        encryptDataECB(original_key, local_key, key_len, local_key, false);
        return local_key;
    }

};

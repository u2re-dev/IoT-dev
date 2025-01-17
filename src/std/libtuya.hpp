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
        esp_aes_context RoundKey;
        esp_aes_init(&RoundKey);
        esp_aes_setkey(&RoundKey, key, 128);
        esp_aes_crypt_cbc(&RoundKey, ESP_AES_DECRYPT, length, iv, data, output);
    }

    //
    void encryptDataCBC(uint8_t* key,  uint8_t* iv,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context RoundKey;
        esp_aes_init(&RoundKey);
        esp_aes_setkey(&RoundKey, key, 128);
        esp_aes_crypt_cbc(&RoundKey, ESP_AES_ENCRYPT, length, iv, data, output);
    }




    //
    void encryptDataECB(uint8_t* key,  uint8_t* iv,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context RoundKey;
        esp_aes_init(&RoundKey);
        esp_aes_setkey(&RoundKey, key, 128);

        // prepare output to encrypt
        memcpy(output, data, length);

        // add post-padding
        const auto pad = ((length + 16 /*- 1*/) >> 4) << 4;
        for (uint I=0;I<pad;I++) {
            output[length+I] = pad;
        }

        //
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&RoundKey, ESP_AES_ENCRYPT, output+I, output+I);
        }

        //
        length += pad; // add padding value
    }

    //
    void decryptDataECB(uint8_t* key,  uint8_t* iv,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context RoundKey;
        esp_aes_init(&RoundKey);
        esp_aes_setkey(&RoundKey, key, 128);

        //
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&RoundKey, ESP_AES_DECRYPT, data+I, output+I);
        }

        // re-correction of length (if possible)
        const auto pad = data[length-1];
        if (pad <= 16 && pad > 0) { length -= pad; };
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
        // protocol 3.4
        const auto encLen = ((length + 15 + 16) >> 4) << 4;
        const auto encOffset = 0;

        // protocol 3.3
        //const auto encLen = ((length + 16) >> 4) << 4;
        //const auto encOffset = 15;

        //
        if (!output) output = (uint8_t*)calloc(1, encLen + encOffset);
        memcpy(output, protocolVersion, 3);
        memcpy(output + 15, data, length);

        // return encryptable zone
        return std::array<uint32_t, 2>{encOffset, encLen};
    }


    //
    void encodeTuyaCode(uint8_t*data, size_t& length,   uint8_t* output, 
        // tuya smart specific
        uint32_t SEQ_NO = 0, 
        uint32_t CMD, 
        uint8_t* hmac
    ) {
        // write header
        *(uint32_t*)(output+0) = 0x000055AA;

        // encode as big-endian
        *(uint32_t*)(output+4)  = bswap32(SEQ_NO);
        *(uint32_t*)(output+8)  = bswap32(CMD);
        *(uint32_t*)(output+12) = bswap32(computePayloadSize(length, hmac ? true : false));

        //
        const auto payload = output+16;
        memcpy(payload, data, length);
        if (hmac) {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, (const unsigned char *) hmac, 16);
            mbedtls_md_hmac_update(&ctx, (const unsigned char *) output, length + 16); // header + payload
            mbedtls_md_hmac_finish(&ctx, payload + length); // write after payload
            mbedtls_md_free(&ctx);
        } else {
            *(uint32_t*)(payload + length) = crc32_be(0, output, length + 16);
        }

        // write suffix
        *(uint32_t*)(output+0) = 0x0000AA55;
    }

};

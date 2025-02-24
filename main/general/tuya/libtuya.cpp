#include "../../crypto/soc.hpp"
#include "./libtuya.hpp"

//
#define AES128 1
#define AES_BLOCKLEN 16

//
namespace tc {

    //
    uint8_t* encryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output, const bool usePadding) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        // prepare output to encrypt
        if (output) memcpy(output, data, length);
        output = output ? output : data;

        // add post-padding
        const auto padded = ((length + 16 /*- 1*/) >> 4) << 4;
        if (usePadding) {
            for (uint I=length;I<padded;I++) {
                output[I] = (padded - length);
            }
        }

        //
        for (uint I=0;I<(usePadding ? padded : length);I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, output+I, output+I);
        }

        // add padding value
        if (usePadding) { length = padded; };
        esp_aes_free(&ctx);

        //
        return output;
    }

    //
    uint8_t* decryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        //
        output = output ? output : data;
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, data+I, output+I);
        }

        // re-correction of length (if possible)
        const auto pad = data[length-1];
        if (pad <= 16 && pad > 0 && length > 16) { length -= pad; };
        esp_aes_free(&ctx);

        //
        return output;
    }



    // HMAC i.e. hmac_key
    size_t prepareTuyaCode(size_t& length, TuyaCmd const& cmdDesc, uint8_t* output) {
        // write header
        *(uint32_t*)(output+0) = bswap32(0x000055AA);

        // encode as big-endian
        const uint32_t header_len = 16;
        const auto payloadSize  = computePayloadSize(length, cmdDesc.HMAC ? true : false);
        *(uint32_t*)(output+4)  = bswap32(cmdDesc.SEQ_NO);
        *(uint32_t*)(output+8)  = bswap32(cmdDesc.CMD_ID);
        *(uint32_t*)(output+12) = bswap32(payloadSize);

        //
        const auto payload = output + header_len;
        for (uint i=0;i<(length + (cmdDesc.HMAC ? 32 : 4));i++) {
            *(payload + i) = 0;
        }

        // write suffix
        *(uint32_t*)(payload + length + (cmdDesc.HMAC ? 32 : 4)) = bswap32(0x0000AA55);

        //
        return payloadSize + header_len;
    }

    // TODO? needs to merge into payload part?
    size_t checksumTuyaCode(uint8_t* code, uint8_t* HMAC) {
        const uint32_t header_len = 16;
        const auto payloadSize = bswap32(*(uint32_t const*)(code+12));
        const auto payload = code + header_len;
        const auto length = payloadSize - ((HMAC ? 32 : 4) + 4);

        //
        for (uint i=0;i<(HMAC ? 32 : 4);i++) {
            *(payload + length + i) = 0;
        }

        //
        if (HMAC) {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, (const unsigned char *) HMAC, 16);
            mbedtls_md_hmac_update(&ctx, (const unsigned char *) code, length + header_len); // header + payload
            mbedtls_md_hmac_finish(&ctx, payload + length); // write after payload
            mbedtls_md_free(&ctx);
        } else {
            *(uint32_t*)(payload + length) = crc32_be(0, code, length + header_len);
        }

        //
        return payloadSize + header_len;
    }

    // HMAC i.e. hmac_key
    size_t encodeTuyaCode(uint8_t* encrypted_data, size_t& length, TuyaCmd const& cmdDesc, uint8_t* output) {
        prepareTuyaCode(length, cmdDesc, output);
        memcpy(output + 16, encrypted_data, length);
        return checksumTuyaCode(output, cmdDesc.HMAC);
    }



    // for protocol 3.4, remote_nonce is encrypted
    uint8_t* encode_remote_hmac(uint8_t* original_key, uint8_t* remote_nonce,  uint8_t* remote_hmac) {
        size_t key_len = 16;
        decryptDataECB(original_key, remote_nonce, key_len, remote_nonce);

        //
        size_t hmac_length = 48;
        remote_hmac = remote_hmac ? remote_hmac : (uint8_t*)calloc(1, hmac_length);

        //
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
        mbedtls_md_hmac_starts(&ctx, (const unsigned char *) original_key, key_len);
        mbedtls_md_hmac_update(&ctx, (const unsigned char *) remote_nonce, key_len);
        mbedtls_md_hmac_finish(&ctx, remote_hmac);
        mbedtls_md_free(&ctx);

        //
        encryptDataECB(original_key, remote_hmac, hmac_length, remote_hmac, false);

        // needs to send these data
        return remote_hmac;
    }

    //
    uint8_t* encode_hmac_key(uint8_t* original_key, uint8_t* remote_nonce,  uint8_t* hmac_payload) {
        size_t key_len = 16;

        //
        hmac_payload = (hmac_payload ? hmac_payload : (uint8_t*)calloc(1, key_len + 12));
        uint8_t* hmac_key = hmac_payload + 12;
        uint32_t* loc = (uint32_t*)hmac_key;

        //
        const uint32_t* loc_n = (uint32_t*)local_nonce;
        const uint32_t* rem_n = (uint32_t*)remote_nonce;
        for (uint8_t I=0;I<4;I++) { loc[I] = loc_n[I]^rem_n[I]; }

        //
        uint8_t* local_key = (uint8_t*)loc; // encode without padding

        //
#ifdef TUYA_35_SUPPORT
        memcpy(hmac_key + key_len + 12, local_nonce, 16);
        encryptDataGCM(original_key, local_key, key_len, local_key, false);
#else
        encryptDataECB(original_key, local_key, key_len, local_key, false);
#endif

        //
        return local_key;
    }

    //
#ifdef TUYA_35_SUPPORT
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
#endif

    // UNUSED
#ifdef ENABLE_UNUSED
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
#endif

    //
#ifdef TUYA_35_SUPPORT
    size_t prepareTuyaCode35(size_t& encrypted_length, TuyaCmd const& cmdDesc, uint8_t* output) {
        // write header
        *(uint32_t*)(output+0) = bswap32(0x00006699);

        // encode as big-endian
        *(uint16_t*)(output+4)  = 0u;
        *(uint32_t*)(output+6)  = bswap32(cmdDesc.SEQ_NO);
        *(uint32_t*)(output+10) = bswap32(cmdDesc.CMD_ID);
        *(uint32_t*)(output+14) = bswap32(encrypted_length);

        // all of those is a part encryption/encoding stage
        // before payload goes 12-bytes IV
        // after payload goes 16-bytes AES-GCM tag
        const uint32_t header_len = 18;
        *(uint32_t*)(output + header_len + encrypted_length) = bswap32(0x00009966);
        return ((encrypted_length + header_len) + 4);
    }

    //
    size_t encodeTuyaCode35(uint8_t* encrypted_data, size_t& encrypted_length, TuyaCmd const& cmdDesc, uint8_t* output) {
        prepareTuyaCode35(encrypted_length, cmdDesc, output);
        memcpy(output + 18, encrypted_data, encrypted_length);
        return ((encrypted_length + 18) + 4);
    }
#endif

};

#include "../libtuya.hpp"

//
#include <esp32s3/rom/crc.h>
#include <mbedtls/md.h>

//
namespace tc {

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

};

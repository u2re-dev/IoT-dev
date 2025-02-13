/* Originally: https://github.com/jedisct1/spake2-ee */

//
#ifndef _C_SPAKE_H
#define _C_SPAKE_H

    //
    #ifdef __cplusplus
    extern "C" {
    #endif
        constexpr size_t crypto_spake_DUMMYKEYBYTES    = 32;
        constexpr size_t crypto_spake_PUBLICDATABYTES  = 36;
        constexpr size_t crypto_spake_RESPONSE1BYTES   = 32;
        constexpr size_t crypto_spake_RESPONSE2BYTES   = 64;
        constexpr size_t crypto_spake_RESPONSE3BYTES   = 32;
        constexpr size_t crypto_spake_SHAREDKEYBYTES   = 32;
        constexpr size_t crypto_spake_STOREDBYTES      = 164;

        //
        static int create_keys(SpakeKeys &keys, unsigned char salt[crypto_pwhash_SALTBYTES], const char *passwd, unsigned long long passwdlen, unsigned long long opslimit, size_t memlimit);
        static int crypto_spake_step4(ServerState &st, SharedKeys &shared_keys, const unsigned char response3[crypto_spake_RESPONSE3BYTES]);
        static int crypto_spake_step0(ServerState &st, unsigned char public_data[crypto_spake_PUBLICDATABYTES], const unsigned char stored_data[crypto_spake_STOREDBYTES]);

        //
        static int crypto_spake_step1(ClientState &st,
            unsigned char response1[crypto_spake_RESPONSE1BYTES],
            const unsigned char public_data[crypto_spake_PUBLICDATABYTES],
            const char *passwd, unsigned long long passwdlen);

        //
        static int crypto_spake_step2(ServerState &st, unsigned char response2[crypto_spake_RESPONSE2BYTES],
            const char *client_id, size_t client_id_len, const char *server_id, size_t server_id_len,
            const unsigned char stored_data[crypto_spake_STOREDBYTES], const unsigned char response1[crypto_spake_RESPONSE1BYTES]);

        //
        static int crypto_spake_step3(ClientState &st,
            unsigned char response3[crypto_spake_RESPONSE3BYTES], SharedKeys &shared_keys,
            const char *client_id, size_t client_id_len, const char *server_id, size_t server_id_len,
            const unsigned char response2[crypto_spake_RESPONSE2BYTES]);

    #ifdef __cplusplus
    }
    #endif

#endif

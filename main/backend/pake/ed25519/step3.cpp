#include "../std/utils.hpp"
#include <sodium.h>

//
int crypto_spake_step3(ClientState &st,
                       unsigned char response3[crypto_spake_RESPONSE3BYTES],
                       SharedKeys &shared_keys,
                       const char *client_id, size_t client_id_len,
                       const char *server_id, size_t server_id_len,
                       const unsigned char response2[crypto_spake_RESPONSE2BYTES])
{
    SpakeValidators validators;
    unsigned char V[32] = {0}, Z[32] = {0};
    unsigned char gy[32] = {0};

    //
    const unsigned char *Y = response2;
    const unsigned char *client_validator_received = response2 + 32;

    //
#ifdef USE_ED25519
    crypto_core_ed25519_sub(gy, Y, st.N.data());
    if(crypto_scalarmult_ed25519_noclamp(Z, st.x.data(), gy) != 0 || crypto_scalarmult_ed25519(V, st.h_L.data(), gy) != 0)
    {
        sodium_memzero(&st, sizeof(st));
        return -1;
    }
#endif

    if (shared_keys_and_validators(shared_keys, validators,
                                   client_id, client_id_len,
                                   server_id, server_id_len,
                                   st.X.data(), Y, Z,
                                   st.h_K.data(), V) != 0)
    {
        sodium_memzero(&st, sizeof(st));
        return -1;
    }

    // Проверка валидатора, полученного от сервера
    if(sodium_memcmp(client_validator_received, validators.client_validator.data(), 32) != 0){
        sodium_memzero(&st, sizeof(st));
        return -1;
    }
    // Отправляем server_validator
    std::memcpy(response3, validators.server_validator.data(), 32);

    sodium_memzero(&st, sizeof(st));
    return 0;
}

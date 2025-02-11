#include "./spake.hpp"
#include "../std/utils.hpp"

//
static int crypto_spake_step2(ServerState &st,
unsigned char response2[crypto_spake_RESPONSE2BYTES],
const char *client_id, size_t client_id_len,
const char *server_id, size_t server_id_len,
const unsigned char stored_data[crypto_spake_STOREDBYTES],
const unsigned char response1[crypto_spake_RESPONSE1BYTES])
{
    SpakeKeys keys;
    SpakeValidators validators;
    unsigned char V[32] = {0}, Z[32] = {0};
    unsigned char gx[32] = {0}, gy[32] = {0};
    unsigned char salt[crypto_pwhash_SALTBYTES] = {0};
    std::array<unsigned char, 32> y_scalar{};
    size_t pos = 0;

    //
    uint16_t ver; pop16(ver, stored_data, pos);

    //
    if (ver != SER_VERSION) return -1;

    //
    uint16_t alg; pop16(alg, stored_data, pos);

    //
    uint64_t opslimit, memlimit;
    pop64(opslimit, stored_data, pos);
    pop64(memlimit, stored_data, pos);

    //
    std::memcpy(salt, stored_data + pos, crypto_pwhash_SALTBYTES);
    pos += crypto_pwhash_SALTBYTES;

    //
    pop256(keys.M.data(), stored_data, pos);
    pop256(keys.N.data(), stored_data, pos);
    pop256(keys.h_K.data(), stored_data, pos);
    pop256(keys.L.data(), stored_data, pos);

    //
    std::array<unsigned char, 32> y;
    random_scalar(y);

#ifdef USE_ED25519
    crypto_scalarmult_ed25519_base(gy, y.data());
    crypto_core_ed25519_add(response2, gy, keys.N.data());
    crypto_core_ed25519_sub(gx, response1, keys.M.data());
    if(crypto_scalarmult_ed25519_noclamp(Z, y.data(), gx) != 0 || crypto_scalarmult_ed25519_noclamp(V, y.data(), keys.L.data()) != 0)
    {
        sodium_memzero(&st, sizeof(st));
        return -1;
    }
#endif

    //
    if (shared_keys_and_validators(st.shared_keys, validators,
    client_id, client_id_len, server_id, server_id_len,
    response1, response2, Z, keys.h_K.data(), V) != 0)
    { sodium_memzero(&st, sizeof(st)); return -1; }

    //
    std::memcpy(response2 + 32, validators.client_validator.data(), 32);
    std::memcpy(st.server_validator.data(), validators.server_validator.data(), 32);
    return 0;
}

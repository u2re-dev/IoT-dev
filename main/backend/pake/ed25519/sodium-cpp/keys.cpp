/* Originally: https://github.com/jedisct1/spake2-ee */
#include "../spake.h"

//
static int create_keys(SpakeKeys &keys, unsigned char salt[crypto_pwhash_SALTBYTES], const char *passwd, unsigned long long passwdlen, unsigned long long opslimit, size_t memlimit)
{
    std::array<unsigned char, 32*4> h_MNKL{};
    unsigned char *h_M = h_MNKL.data() + (0*32);
    unsigned char *h_N = h_MNKL.data() + (1*32);
    unsigned char *h_K = h_MNKL.data() + (2*32);
    unsigned char *h_L = h_MNKL.data() + (3*32);

    //
    if (crypto_pwhash(h_MNKL.data(), h_MNKL.size(), passwd, passwdlen, salt, opslimit, memlimit, crypto_pwhash_alg_default()) != 0)
    { return -1; }

#ifdef USE_ED25519
    crypto_core_ed25519_from_uniform(keys.M.data(), h_M);
    crypto_core_ed25519_from_uniform(keys.N.data(), h_N);
#endif

    //
    memcpy(keys.h_K.data(), h_K, 32);
    memcpy(keys.h_L.data(), h_L, 32);

    //
#ifdef USE_ED25519
    crypto_scalarmult_ed25519_base(keys.L.data(), keys.h_L.data());
#endif

    //
    return 0;
}

//
static int crypto_spake_server_store(unsigned char stored_data[crypto_spake_STOREDBYTES],
const char *passwd, unsigned long long passwdlen, unsigned long long opslimit, size_t memlimit)
{
    SpakeKeys keys;
    unsigned char salt[crypto_pwhash_SALTBYTES];
    size_t pos = 0;

    //
    randombytes_buf(salt, sizeof salt);
    if (create_keys(keys, salt, passwd, passwdlen, opslimit, memlimit) != 0) return -1;

    //
    push16(stored_data, pos, SER_VERSION);
    push16(stored_data, pos, static_cast<uint16_t>(crypto_pwhash_alg_default()));
    push64(stored_data, pos, opslimit);
    push64(stored_data, pos, memlimit);

    //
    memcpy(stored_data + pos, salt, crypto_pwhash_SALTBYTES); pos += crypto_pwhash_SALTBYTES;

    //
    push256(stored_data, pos, keys.M.data());
    push256(stored_data, pos, keys.N.data());
    push256(stored_data, pos, keys.h_K.data());
    push256(stored_data, pos, keys.L.data());

    //
    assert(pos == crypto_spake_STOREDBYTES);
    return 0;
}

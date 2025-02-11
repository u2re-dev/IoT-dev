#include "./spake.hpp"
#include "../std/utils.hpp"

//
#include <sodium.h>

//
static int crypto_spake_step1(ClientState &st,
unsigned char response1[crypto_spake_RESPONSE1BYTES],
const unsigned char public_data[crypto_spake_PUBLICDATABYTES],
const char *passwd, unsigned long long passwdlen)
{
    SpakeKeys keys;
    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
    std::array<unsigned char, 32> x{};
    unsigned char gx[32] = {0};
    size_t pos = 0;

    //
    uint16_t ver; pop16(ver, public_data, pos);
    if (ver != SER_VERSION) return -1;

    //
    uint16_t alg; pop16(alg, public_data, pos);
    uint64_t opslimit, memlimit;
    pop64(opslimit, public_data, pos);
    pop64(memlimit, public_data, pos);

    //
    std::memcpy(salt.data(), public_data + pos, crypto_pwhash_SALTBYTES);
    pos += crypto_pwhash_SALTBYTES;

    //
    if (create_keys(keys, salt.data(), passwd, passwdlen, opslimit, memlimit) != 0) {
        sodium_memzero(&st, sizeof(st));
        return -1;
    }

    //
    std::array<unsigned char, 32> x_scalar;
    random_scalar(x_scalar);
    std::array<unsigned char, 32> gx_arr = {0};

#ifdef USE_ED25519
    crypto_scalarmult_ed25519_base(gx_arr.data(), x_scalar.data());
    crypto_core_ed25519_add(response1, gx_arr.data(), keys.M.data());
#endif

    // Сохраняем необходимые данные в состоянии
    std::memcpy(st.h_K.data(), keys.h_K.data(), 32);
    std::memcpy(st.h_L.data(), keys.h_L.data(), 32);
    std::memcpy(st.N.data(), keys.N.data(), 32);
    std::memcpy(st.x.data(), x_scalar.data(), 32);
    std::memcpy(st.X.data(), response1, 32);
    return 0;
}

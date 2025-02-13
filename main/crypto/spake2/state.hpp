#pragma once

//
#include "../../std/types.hpp"

//
#ifdef ENABLE_PAKE_STATE
struct SpakeKeys {
    std::array<uint8_t, 32> M;
    std::array<uint8_t, 32> N;
    std::array<uint8_t, 32> L;
    std::array<uint8_t, 32> h_K;
    std::array<uint8_t, 32> h_L;
};

//
struct SpakeValidators {
    std::array<uint8_t, 32> client_validator;
    std::array<uint8_t, 32> server_validator;
};

//
struct SharedKeys {
    std::array<uint8_t, crypto_spake_SHAREDKEYBYTES> client_sk;
    std::array<uint8_t, crypto_spake_SHAREDKEYBYTES> server_sk;
};

//
struct ClientState {
    std::array<uint8_t, 32> h_K;
    std::array<uint8_t, 32> h_L;
    std::array<uint8_t, 32> N;
    std::array<uint8_t, 32> x; // secret scalar
    std::array<uint8_t, 32> X; // public
};

//
struct ServerState {
    SharedKeys shared_keys;
    std::array<uint8_t, 32> server_validator;
};

//
static int crypto_spake_server_store(uint8_t stored_data[crypto_spake_STOREDBYTES], const char *passwd, unsigned long long passwdlen, unsigned long long opslimit, size_t memlimit)
{
    SpakeKeys keys;
    uint8_t salt[crypto_pwhash_SALTBYTES];
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
#endif

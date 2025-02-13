#pragma once

//
#ifdef ENABLE_PAKE_STATE
struct SpakeKeys {
    std::array<unsigned char, 32> M;
    std::array<unsigned char, 32> N;
    std::array<unsigned char, 32> L;
    std::array<unsigned char, 32> h_K;
    std::array<unsigned char, 32> h_L;
};

//
struct SpakeValidators {
    std::array<unsigned char, 32> client_validator;
    std::array<unsigned char, 32> server_validator;
};

//
struct SharedKeys {
    std::array<unsigned char, crypto_spake_SHAREDKEYBYTES> client_sk;
    std::array<unsigned char, crypto_spake_SHAREDKEYBYTES> server_sk;
};

//
struct ClientState {
    std::array<unsigned char, 32> h_K;
    std::array<unsigned char, 32> h_L;
    std::array<unsigned char, 32> N;
    std::array<unsigned char, 32> x; // secret scalar
    std::array<unsigned char, 32> X; // public
};

//
struct ServerState {
    SharedKeys shared_keys;
    std::array<unsigned char, 32> server_validator;
};

//
static int crypto_spake_server_store(unsigned char stored_data[crypto_spake_STOREDBYTES], const char *passwd, unsigned long long passwdlen, unsigned long long opslimit, size_t memlimit)
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
#endif

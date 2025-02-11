#include "../std/utils.hpp"

//
int crypto_spake_step0(ServerState &st, unsigned char public_data[crypto_spake_PUBLICDATABYTES], const unsigned char stored_data[crypto_spake_STOREDBYTES])
{
    size_t pos_in = 0, pos_out = 0;
    uint16_t ver; pop16(ver, stored_data, pos_in);
    if (ver != SER_VERSION) return -1;
    push16(public_data, pos_out, ver);

    //
    uint16_t alg;
    pop16(alg, stored_data, pos_in);
    push16(public_data, pos_out, alg);

    //
    uint64_t opslimit, memlimit;
    pop64(opslimit, stored_data, pos_in);
    push64(public_data, pos_out, opslimit);
    pop64(memlimit, stored_data, pos_in);
    push64(public_data, pos_out, memlimit);

    //
    unsigned char salt[crypto_pwhash_SALTBYTES];
    std::memcpy(salt,  stored_data + pos_in, crypto_pwhash_SALTBYTES); pos_in  += crypto_pwhash_SALTBYTES;
    std::memcpy(public_data + pos_out, salt, crypto_pwhash_SALTBYTES); pos_out += crypto_pwhash_SALTBYTES;
    assert(pos_out == crypto_spake_PUBLICDATABYTES);
    return 0;
}

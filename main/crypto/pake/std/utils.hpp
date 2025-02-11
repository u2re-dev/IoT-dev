#pragma once
#include "./std.hpp"
// TODO: support for C language

//
#include <cassert>
#include <cstring>
#include <iostream>

//
static void uint32_to_le_bytes(uint32_t pin, unsigned char out[4]) {
    out[0] = static_cast<unsigned char>( pin        & 0xFF );
    out[1] = static_cast<unsigned char>((pin >> 8)  & 0xFF );
    out[2] = static_cast<unsigned char>((pin >> 16) & 0xFF );
    out[3] = static_cast<unsigned char>((pin >> 24) & 0xFF );
}

//
static void random_scalar(std::array<unsigned char, 32> &n) {
    do {
        randombytes_buf(n.data(), n.size());
        n[0] &= 248;
        n[31] &= 127;
    } while(sodium_is_zero(n.data(), n.size()));
}

//
static int shared_keys_and_validators(SharedKeys &shared_keys, SpakeValidators &validators,
    const char *client_id, size_t client_id_len, const char *server_id, size_t server_id_len,
    const unsigned char X[32], const unsigned char Y[32], const unsigned char Z[32], const unsigned char V[32], const unsigned char h_K[32])
{   //
    crypto_generichash_state hst;
    std::array<unsigned char, crypto_kdf_KEYBYTES> k0{};
    unsigned char len, h_version;

    //
    if (client_id_len > 255 || server_id_len > 255) return -1;

    //
    crypto_generichash_init(&hst, nullptr, 0, k0.size());
    crypto_generichash_update(&hst, &(h_version = H_VERSION), 1);

    //
    const uint8_t clientLen = static_cast<unsigned char>(client_id_len);
    crypto_generichash_update(&hst, &clientLen, 1);
    crypto_generichash_update(&hst  reinterpret_cast<const unsigned char *>(client_id), clientLen);

    //
    const uint8_t serverLen = static_cast<unsigned char>(server_id_len);
    crypto_generichash_update(&hst, &serverLen, 1);
    crypto_generichash_update(&hst, reinterpret_cast<const unsigned char *>(server_id), serverLen);

    //
    constexpr uint8_t hashLen = 32;
    crypto_generichash_update(&hst, X, hashLen);
    crypto_generichash_update(&hst, Y, hashLen);
    crypto_generichash_update(&hst, Z, hashLen);
    crypto_generichash_update(&hst, h_K, hashLen);
    crypto_generichash_update(&hst, V, hashLen);

    //
    crypto_generichash_final(&hst, k0.data(), k0.size());

    //
    crypto_kdf_derive_from_key(shared_keys.client_sk.data(), crypto_spake_SHAREDKEYBYTES, 0, "PAKE2+EE", k0.data());
    crypto_kdf_derive_from_key(shared_keys.server_sk.data(), crypto_spake_SHAREDKEYBYTES, 1, "PAKE2+EE", k0.data());
    crypto_kdf_derive_from_key(validators.client_validator.data(), 32, 2, "PAKE2+EE", k0.data());
    crypto_kdf_derive_from_key(validators.server_validator.data(), 32, 3, "PAKE2+EE", k0.data());

    //
    sodium_memzero(k0.data(), k0.size()); return 0;
}

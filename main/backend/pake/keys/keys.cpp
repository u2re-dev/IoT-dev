
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
{
    crypto_generichash_state hst;
    std::array<unsigned char, crypto_kdf_KEYBYTES> k0{};
    unsigned char len, h_version;

    if (client_id_len > 255 || server_id_len > 255)
        return -1;
    crypto_generichash_init(&hst, nullptr, 0, k0.size());

    h_version = H_VERSION;
    crypto_generichash_update(&hst, &h_version, 1);

    len = static_cast<unsigned char>(client_id_len);
    crypto_generichash_update(&hst, &len, 1);
    crypto_generichash_update(&hst  reinterpret_cast<const unsigned char *>(client_id),  len);

    len = static_cast<unsigned char>(server_id_len);
    crypto_generichash_update(&hst, &len, 1);
    crypto_generichash_update(&hst, reinterpret_cast<const unsigned char *>(server_id), len);

    len = 32;
    crypto_generichash_update(&hst, X, len);
    crypto_generichash_update(&hst, Y, len);
    crypto_generichash_update(&hst, Z, len);
    crypto_generichash_update(&hst, h_K, len);
    crypto_generichash_update(&hst, V, len);

    //
    crypto_generichash_final(&hst, k0.data(), k0.size());

    //
    crypto_kdf_derive_from_key(shared_keys.client_sk.data(), crypto_spake_SHAREDKEYBYTES, 0, "PAKE2+EE", k0.data());
    crypto_kdf_derive_from_key(shared_keys.server_sk.data(), crypto_spake_SHAREDKEYBYTES, 1, "PAKE2+EE", k0.data());
    crypto_kdf_derive_from_key(validators.client_validator.data(), 32, 2, "PAKE2+EE", k0.data());
    crypto_kdf_derive_from_key(validators.server_validator.data(), 32, 3, "PAKE2+EE", k0.data());
    sodium_memzero(k0.data(), k0.size());
    return 0;
}

//
int crypto_spake_server_store(unsigned char stored_data[crypto_spake_STOREDBYTES],
                              const char *passwd, unsigned long long passwdlen,
                              unsigned long long opslimit, size_t memlimit)
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
    std::memcpy(stored_data + pos, salt, crypto_pwhash_SALTBYTES); pos += crypto_pwhash_SALTBYTES;

    //
    push256(stored_data, pos, keys.M.data());
    push256(stored_data, pos, keys.N.data());
    push256(stored_data, pos, keys.h_K.data());
    push256(stored_data, pos, keys.L.data());

    //
    assert(pos == crypto_spake_STOREDBYTES);
    return 0;
}

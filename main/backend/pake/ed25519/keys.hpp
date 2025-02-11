//
static int create_keys(SpakeKeys &keys, unsigned char salt[crypto_pwhash_SALTBYTES], const char *passwd, unsigned long long passwdlen, unsigned long long opslimit, size_t memlimit)
{
    // Будем вычислять 4 блока по 32 байта
    std::array<unsigned char, 32*4> h_MNKL{};
    unsigned char *h_M = h_MNKL.data() + (0*32);
    unsigned char *h_N = h_MNKL.data() + (1*32);
    unsigned char *h_K = h_MNKL.data() + (2*32);
    unsigned char *h_L = h_MNKL.data() + (3*32);

    if (crypto_pwhash(h_MNKL.data(), h_MNKL.size(), passwd, passwdlen, salt, opslimit, memlimit, crypto_pwhash_alg_default()) != 0) {
        return -1;
    }

#ifdef USE_ED25519
    crypto_core_ed25519_from_uniform(keys.M.data(), h_M);
    crypto_core_ed25519_from_uniform(keys.N.data(), h_N);
#endif

    //
    std::memcpy(keys.h_K.data(), h_K, 32);
    std::memcpy(keys.h_L.data(), h_L, 32);

    //
#ifdef USE_ED25519
    crypto_scalarmult_ed25519_base(keys.L.data(), keys.h_L.data());
#endif

    return 0;
}

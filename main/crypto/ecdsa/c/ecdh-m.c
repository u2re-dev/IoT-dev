
/**********************************************************************
 *
 * ECDH
 *
 **********************************************************************/


/*
 * ECDH/ECDSA generate pair
 */
int p256_gen_keypair(uint8_t priv[32], uint8_t pub[64])
{
    uint32_t s[8], x[8], y[8];
    int ret = scalar_gen_with_pub(priv, s, x, y);
    zeroize(s, sizeof s);
    if (ret != 0)
        return P256_RANDOM_FAILED;

    point_to_bytes(pub, x, y);
    return 0;
}

/*
 * ECDH compute shared secret
 */
int p256_ecdh_shared_secret(uint8_t secret[32],
                            const uint8_t priv[32], const uint8_t peer[64])
{
    CT_POISON(priv, 32);

    uint32_t s[8], px[8], py[8], x[8], y[8];
    int ret;

    ret = scalar_from_bytes(s, priv);
    CT_UNPOISON(&ret, sizeof ret);
    if (ret != 0) {
        ret = P256_INVALID_PRIVKEY;
        goto cleanup;
    }

    ret = point_from_bytes(px, py, peer);
    if (ret != 0) {
        ret = P256_INVALID_PUBKEY;
        goto cleanup;
    }

    scalar_mult(x, y, px, py, s);

    m256_to_bytes(secret, x, &p256_p);
    CT_UNPOISON(secret, 32);

cleanup:
    zeroize(s, sizeof s);
    return ret;
}

#ifdef ENABLE_ECC_M

/**********************************************************************
 *
 * ECDSA
 *
 * Reference:
 * [SEC1] SEC 1: Elliptic Curve Cryptography, Certicom research, 2009.
 *        http://www.secg.org/sec1-v2.pdf
 **********************************************************************/

/*
 * Reduction mod n of a small number
 *
 * in: x in [0, 2^256)
 * out: x_out = x_in mod n in [0, n)
 */
static void ecdsa_m256_mod_n(uint32_t x[8])
{
    uint32_t t[8];
    uint32_t c = u256_sub(t, x, p256_n.m);
    u256_cmov(x, t, 1 - c);
}

/*
 * Import integer mod n (Montgomery domain) from hash
 *
 * in: h = h0, ..., h_hlen
 *     hlen the length of h in bytes
 * out: z = (h0 * 2^l-8 + ... + h_l) * 2^256 mod n
 *      with l = min(32, hlen)
 *
 * Note: in [SEC1] this is step 5 of 4.1.3 (sign) or step 3 or 4.1.4 (verify),
 * with obvious simplications since n's bit-length is a multiple of 8.
 */
static void ecdsa_m256_from_hash(uint32_t z[8],
                                 const uint8_t *h, size_t hlen)
{
    /* convert from h (big-endian) */
    /* hlen is public data so it's OK to branch on it */
    if (hlen < 32) {
        uint8_t p[32];
        for (unsigned i = 0; i < 32; i++)
            p[i] = 0;
        for (unsigned i = 0; i < hlen; i++)
            p[32 - hlen + i] = h[i];
        u256_from_bytes(z, p);
    } else {
        u256_from_bytes(z, h);
    }

    /* ensure the result is in [0, n) */
    ecdsa_m256_mod_n(z);

    /* map to Montgomery domain */
    m256_prep(z, &p256_n);
}

/*
 * ECDSA sign
 */
int p256_ecdsa_sign(uint8_t sig[64], const uint8_t priv[32],
                    const uint8_t *hash, size_t hlen)
{
    CT_POISON(priv, 32);

    /*
     * Steps and notations from [SEC1] 4.1.3
     *
     * Instead of retrying on r == 0 or s == 0, just abort,
     * as those events have negligible probability.
     */
    int ret;

    /* Temporary buffers - the first two are mostly stable, so have names */
    uint32_t xr[8], k[8], t3[8], t4[8];

    /* 1. Set ephemeral keypair */
    uint8_t *kb = (uint8_t *) t4;
    /* kb will be erased by re-using t4 for dU - if we exit before that, we
     * haven't read the private key yet so we kb isn't sensitive yet */
    ret = scalar_gen_with_pub(kb, k, xr, t3);   /* xr = x_coord(k * G) */
    if (ret != 0)
        return P256_RANDOM_FAILED;
    m256_prep(k, &p256_n);

    /* 2. Convert xr to an integer */
    m256_done(xr, &p256_p);

    /* 3. Reduce xr mod n (extra: output it while at it) */
    ecdsa_m256_mod_n(xr);    /* xr = int(xr) mod n */

    /* xr is public data so it's OK to use a branch */
    if (u256_diff0(xr) == 0)
        return P256_RANDOM_FAILED;

    u256_to_bytes(sig, xr);

    m256_prep(xr, &p256_n);

    /* 4. Skipped - we take the hash as an input, not the message */

    /* 5. Derive an integer from the hash */
    ecdsa_m256_from_hash(t3, hash, hlen);   /* t3 = e */

    /* 6. Compute s = k^-1 * (e + r * dU) */

    /* Note: dU will be erased by re-using t4 for the value of s (public) */
    ret = scalar_from_bytes(t4, priv);   /* t4 = dU (integer domain) */
    CT_UNPOISON(&ret, sizeof ret); /* Result of input validation */
    if (ret != 0)
        return P256_INVALID_PRIVKEY;
    m256_prep(t4, &p256_n);         /* t4 = dU (Montgomery domain) */

    m256_inv(k, k, &p256_n);        /* k^-1 */
    m256_mul(t4, xr, t4, &p256_n);  /* t4 = r * dU */
    m256_add(t4, t3, t4, &p256_n);  /* t4 = e + r * dU */
    m256_mul(t4, k, t4, &p256_n);   /* t4 = s = k^-1 * (e + r * dU) */
    zeroize(k, sizeof k);

    /* 7. Output s (r already outputed at step 3) */
    CT_UNPOISON(t4, 32);
    if (u256_diff0(t4) == 0) {
        /* undo early output of r */
        u256_to_bytes(sig, t4);
        return P256_RANDOM_FAILED;
    }
    m256_to_bytes(sig + 32, t4, &p256_n);

    return P256_SUCCESS;
}

/*
 * ECDSA verify
 */
int p256_ecdsa_verify(const uint8_t sig[64], const uint8_t pub[64],
                      const uint8_t *hash, size_t hlen)
{
    /*
     * Steps and notations from [SEC1] 4.1.3
     *
     * Note: we're using public data only, so branches are OK
     */
    int ret;

    /* 1. Validate range of r and s : [1, n-1] */
    uint32_t r[8], s[8];
    ret = scalar_from_bytes(r, sig);
    if (ret != 0)
        return P256_INVALID_SIGNATURE;
    ret = scalar_from_bytes(s, sig + 32);
    if (ret != 0)
        return P256_INVALID_SIGNATURE;

    /* 2. Skipped - we take the hash as an input, not the message */

    /* 3. Derive an integer from the hash */
    uint32_t e[8];
    ecdsa_m256_from_hash(e, hash, hlen);

    /* 4. Compute u1 = e * s^-1 and u2 = r * s^-1 */
    uint32_t u1[8], u2[8];
    m256_prep(s, &p256_n);           /* s in Montgomery domain */
    m256_inv(s, s, &p256_n);         /* s = s^-1 mod n */
    m256_mul(u1, e, s, &p256_n);     /* u1 = e * s^-1 mod n */
    m256_done(u1, &p256_n);          /* u1 out of Montgomery domain */

    u256_cmov(u2, r, 1);
    m256_prep(u2, &p256_n);          /* r in Montgomery domain */
    m256_mul(u2, u2, s, &p256_n);    /* u2 = r * s^-1 mod n */
    m256_done(u2, &p256_n);          /* u2 out of Montgomery domain */

    /* 5. Compute R (and re-use (u1, u2) to store its coordinates */
    uint32_t px[8], py[8];
    ret = point_from_bytes(px, py, pub);
    if (ret != 0)
        return P256_INVALID_PUBKEY;

    scalar_mult(e, s, px, py, u2);      /* (e, s) = R2 = u2 * Qu */

    if (u256_diff0(u1) == 0) {
        /* u1 out of range for scalar_mult() - just skip it */
        u256_cmov(u1, e, 1);
        /* we don't care about the y coordinate */
    } else {
        scalar_mult(px, py, p256_gx, p256_gy, u1); /* (px, py) = R1 = u1 * G */

        /* (u1, u2) = R = R1 + R2 */
        point_add_or_double_leaky(u1, u2, px, py, e, s);
        /* No need to check if R == 0 here: if that's the case, it will be
         * caught when comparating rx (which will be 0) to r (which isn't). */
    }

    /* 6. Convert xR to an integer */
    m256_done(u1, &p256_p);

    /* 7. Reduce xR mod n */
    ecdsa_m256_mod_n(u1);

    /* 8. Compare xR mod n to r */
    uint32_t diff = u256_diff(u1, r);
    if (diff == 0)
        return P256_SUCCESS;

    return P256_INVALID_SIGNATURE;
}
#endif

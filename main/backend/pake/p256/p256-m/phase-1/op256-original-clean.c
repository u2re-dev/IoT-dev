/*
 * Implementation of curve P-256 (ECDH and ECDSA)
 * Originally: https://github.com/mpg/p256-m
 *
 * Author: Manuel Pégourié-Gonnard.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "p256-m.h"

static void zeroize(void *d, size_t n)
{
    volatile char *p = d;
    while( n-- )
        *p++ = 0;
}

#if defined(CT_MEMSAN)

#include <sanitizer/msan_interface.h>
#define CT_POISON   __msan_allocated_memory
#define CT_UNPOISON __msan_unpoison

#elif defined(CT_VALGRIND)

#include <valgrind/memcheck.h>
#define CT_POISON   VALGRIND_MAKE_MEM_UNDEFINED
#define CT_UNPOISON VALGRIND_MAKE_MEM_DEFINED

#else
#define CT_POISON(p, sz)
#define CT_UNPOISON(p, sz)
#endif


static void u288_rshift32(uint32_t z[9], uint32_t c)
{
    for (unsigned i = 0; i < 8; i++) {
        z[i] = z[i + 1];
    }
    z[8] = c;
}



static void u256_set32(uint32_t z[8], uint32_t x)
{
    z[0] = x;
    for (unsigned i = 1; i < 8; i++) {
        z[i] = 0;
    }
}

static uint32_t u256_add(uint32_t z[8], const uint32_t x[8], const uint32_t y[8])
{
    uint32_t carry = 0;

    for (unsigned i = 0; i < 8; i++) {
        uint64_t sum = (uint64_t) carry + x[i] + y[i];
        z[i] = (uint32_t) sum;
        carry = (uint32_t) (sum >> 32);
    }

    return carry;
}

static uint32_t u256_sub(uint32_t z[8], const uint32_t x[8], const uint32_t y[8])
{
    uint32_t carry = 0;

    for (unsigned i = 0; i < 8; i++) {
        uint64_t diff = (uint64_t) x[i] - y[i] - carry;
        z[i] = (uint32_t) diff;
        carry = -(uint32_t) (diff >> 32);
    }

    return carry;
}

static void u256_cmov(uint32_t z[8], const uint32_t x[8], uint32_t c)
{
    const uint32_t x_mask = -c;
    for (unsigned i = 0; i < 8; i++) {
        z[i] = (z[i] & ~x_mask) | (x[i] & x_mask);
    }
}

static uint32_t u256_diff(const uint32_t x[8], const uint32_t y[8])
{
    uint32_t diff = 0;
    for (unsigned i = 0; i < 8; i++) {
        diff |= x[i] ^ y[i];
    }
    return diff;
}

static uint32_t u256_diff0(const uint32_t x[8])
{
    uint32_t diff = 0;
    for (unsigned i = 0; i < 8; i++) {
        diff |= x[i];
    }
    return diff;
}

static void u256_from_bytes(uint32_t z[8], const uint8_t p[32])
{
    for (unsigned i = 0; i < 8; i++) {
        unsigned j = 4 * (7 - i);
        z[i] = ((uint32_t) p[j + 0] << 24) |
               ((uint32_t) p[j + 1] << 16) |
               ((uint32_t) p[j + 2] <<  8) |
               ((uint32_t) p[j + 3] <<  0);
    }
}

static void u256_to_bytes(uint8_t p[32], const uint32_t z[8])
{
    for (unsigned i = 0; i < 8; i++) {
        unsigned j = 4 * (7 - i);
        p[j + 0] = (uint8_t) (z[i] >> 24);
        p[j + 1] = (uint8_t) (z[i] >> 16);
        p[j + 2] = (uint8_t) (z[i] >>  8);
        p[j + 3] = (uint8_t) (z[i] >>  0);
    }
}

static void m256_add(uint32_t z[8], const uint32_t x[8], const uint32_t y[8], const m256_mod *mod)
{
    uint32_t r[8];
    uint32_t carry_add = u256_add(z, x, y);
    uint32_t carry_sub = u256_sub(r, z, mod->m);
    uint32_t use_sub = carry_add | (1 - carry_sub);
    u256_cmov(z, r, use_sub);
}

static void m256_add_p(uint32_t z[8], const uint32_t x[8], const uint32_t y[8])
{
    m256_add(z, x, y, &p256_p);
}

static void m256_sub(uint32_t z[8], const uint32_t x[8], const uint32_t y[8], const m256_mod *mod)
{
    uint32_t r[8];
    uint32_t carry = u256_sub(z, x, y);
    (void) u256_add(r, z, mod->m);
    u256_cmov(z, r, carry);
}

static void m256_sub_p(uint32_t z[8], const uint32_t x[8], const uint32_t y[8])
{
    m256_sub(z, x, y, &p256_p);
}

static void m256_mul(uint32_t z[8], const uint32_t x[8], const uint32_t y[8], const m256_mod *mod)
{
    uint32_t m_prime = mod->ni;
    uint32_t a[9];

    for (unsigned i = 0; i < 9; i++) {
        a[i] = 0;
    }

    for (unsigned i = 0; i < 8; i++) {
        uint32_t u = (a[0] + x[i] * y[0]) * m_prime;

        uint32_t c = u288_muladd(a, x[i], y);
        c += u288_muladd(a, u, mod->m);
        u288_rshift32(a, c);
    }

    uint32_t carry_add = a[8];
    uint32_t carry_sub = u256_sub(z, a, mod->m);
    uint32_t use_sub = carry_add | (1 - carry_sub);
    u256_cmov(z, a, 1 - use_sub);
}

static void m256_mul_p(uint32_t z[8], const uint32_t x[8], const uint32_t y[8])
{
    m256_mul(z, x, y, &p256_p);
}

static void m256_prep(uint32_t z[8], const m256_mod *mod)
{
    m256_mul(z, z, mod->R2, mod);
}

static void m256_done(uint32_t z[8], const m256_mod *mod)
{
    uint32_t one[8];
    u256_set32(one, 1);
    m256_mul(z, z, one, mod);
}

static void m256_set32(uint32_t z[8], uint32_t x, const m256_mod *mod)
{
    u256_set32(z, x);
    m256_prep(z, mod);
}

static void m256_inv(uint32_t z[8], const uint32_t x[8], const m256_mod *mod)
{
    uint32_t bitval[8];
    u256_cmov(bitval, x, 1);

    m256_set32(z, 1, mod);

    unsigned i = 0;
    uint32_t limb = mod->m[i] - 2;
    while (1) {
        for (unsigned j = 0; j < 32; j++) {
            if ((limb & 1) != 0) {
                m256_mul(z, z, bitval, mod);
            }
            m256_mul(bitval, bitval, bitval, mod);
            limb >>= 1;
        }

        if (i == 7)
            break;

        i++;
        limb = mod->m[i];
    }
}

static int m256_from_bytes(uint32_t z[8], const uint8_t p[32], const m256_mod *mod)
{
    u256_from_bytes(z, p);

    uint32_t t[8];
    uint32_t lt_m = u256_sub(t, z, mod->m);
    if (lt_m != 1)
        return -1;

    m256_prep(z, mod);
    return 0;
}

static void m256_to_bytes(uint8_t p[32], const uint32_t z[8], const m256_mod *mod)
{
    uint32_t zi[8];
    u256_cmov(zi, z, 1);
    m256_done(zi, mod);

    u256_to_bytes(p, zi);
}


static uint32_t point_check(const uint32_t x[8], const uint32_t y[8])
{
    uint32_t lhs[8], rhs[8];

    m256_mul_p(lhs, y, y);

    m256_mul_p(rhs, x,   x);
    m256_mul_p(rhs, rhs, x);
    for (unsigned i = 0; i < 3; i++)
        m256_sub_p(rhs, rhs, x);
    m256_add_p(rhs, rhs, p256_b);

    return u256_diff(lhs, rhs);
}

static void point_to_affine(uint32_t x[8], uint32_t y[8], uint32_t z[8])
{
    uint32_t t[8];

    m256_inv(z, z, &p256_p);

    m256_mul_p(t, z, z);
    m256_mul_p(x, x, t);

    m256_mul_p(t, t, z);
    m256_mul_p(y, y, t);
}

static void point_double(uint32_t x[8], uint32_t y[8], uint32_t z[8])
{
    uint32_t m[8], s[8], u[8];

    m256_mul_p(s, z, z);
    m256_add_p(m, x, s);
    m256_sub_p(u, x, s);
    m256_mul_p(s, m, u);
    m256_add_p(m, s, s);
    m256_add_p(m, m, s);

    m256_mul_p(u, y, y);
    m256_add_p(u, u, u);
    m256_mul_p(s, x, u);
    m256_add_p(s, s, s);

    m256_mul_p(u, u, u);
    m256_add_p(u, u, u);

    m256_mul_p(x, m, m);
    m256_sub_p(x, x, s);
    m256_sub_p(x, x, s);

    m256_mul_p(z, y, z);
    m256_add_p(z, z, z);

    m256_sub_p(y, s, x);
    m256_mul_p(y, y, m);
    m256_sub_p(y, y, u);
}

static void point_add(uint32_t x1[8], uint32_t y1[8], uint32_t z1[8], const uint32_t x2[8], const uint32_t y2[8])
{
    uint32_t t1[8], t2[8], t3[8];


    m256_mul_p(t1, z1, z1);
    m256_mul_p(t2, t1, z1);
    m256_mul_p(t1, t1, x2);

    m256_mul_p(t2, t2, y2);

    m256_sub_p(t1, t1, x1);

    m256_sub_p(t2, t2, y1);

    m256_mul_p(z1, z1, t1);

    m256_mul_p(t3, t1, t1);
    m256_mul_p(t1, t3, t1);

    m256_mul_p(t3, t3, x1);

    m256_mul_p(x1, t2, t2);
    m256_sub_p(x1, x1, t3);
    m256_sub_p(x1, x1, t3);
    m256_sub_p(x1, x1, t1);

    m256_sub_p(t3, t3, x1);
    m256_mul_p(t3, t3, t2);
    m256_mul_p(t1, t1, y1);
    m256_sub_p(y1, t3, t1);
}

static void point_add_or_double_leaky(uint32_t x3[8], uint32_t y3[8], const uint32_t x1[8], const uint32_t y1[8], const uint32_t x2[8], const uint32_t y2[8])
{

    uint32_t z3[8];
    u256_cmov(x3, x1, 1);
    u256_cmov(y3, y1, 1);
    m256_set32(z3, 1, &p256_p);

    if (u256_diff(x1, x2) != 0) {
        point_add(x3, y3, z3, x2, y2);
        point_to_affine(x3, y3, z3);
    }
    else if (u256_diff(y1, y2) == 0) {
        point_double(x3, y3, z3);
        point_to_affine(x3, y3, z3);
    } else {
        m256_set32(x3, 0, &p256_p);
        m256_set32(y3, 0, &p256_p);
    }
}

static int point_from_bytes(uint32_t x[8], uint32_t y[8], const uint8_t p[64])
{
    int ret;

    ret = m256_from_bytes(x, p, &p256_p);
    if (ret != 0)
        return ret;

    ret = m256_from_bytes(y, p + 32, &p256_p);
    if (ret != 0)
        return ret;

    return (int) point_check(x, y);
}

static void point_to_bytes(uint8_t p[64], const uint32_t x[8], const uint32_t y[8])
{
    m256_to_bytes(p,        x, &p256_p);
    m256_to_bytes(p + 32,   y, &p256_p);
}


static void scalar_mult(uint32_t rx[8], uint32_t ry[8], const uint32_t px[8], const uint32_t py[8], const uint32_t s[8])
{
    uint32_t s_odd[8], py_neg[8], py_use[8], rz[8];

    u256_sub(s_odd, p256_n.m, s);
    uint32_t negate = ~s[0] & 1;
    u256_cmov(s_odd, s, 1 - negate);

    u256_set32(py_use, 0);
    m256_sub_p(py_neg, py_use, py);

    u256_cmov(rx, px, 1);
    u256_cmov(ry, py, 1);
    m256_set32(rz, 1, &p256_p);
    u256_cmov(ry, py_neg, negate);

    for (unsigned i = 255; i > 0; i--) {
        uint32_t bit = (s_odd[i / 32] >> i % 32) & 1;

        u256_cmov(py_use, py, bit ^ negate);
        u256_cmov(py_use, py_neg, (1 - bit) ^ negate);

        point_double(rx, ry, rz);
        point_add(rx, ry, rz, px, py_use);
    }

    point_to_affine(rx, ry, rz);
}

static int scalar_from_bytes(uint32_t s[8], const uint8_t p[32])
{
    u256_from_bytes(s, p);

    uint32_t r[8];
    uint32_t lt_n = u256_sub(r, s, p256_n.m);

    u256_set32(r, 1);
    uint32_t lt_1 = u256_sub(r, s, r);

    if (lt_n && !lt_1)
        return 0;

    return -1;
}

static int scalar_gen_with_pub(uint8_t sbytes[32], uint32_t s[8], uint32_t x[8], uint32_t y[8])
{
    int ret;
    unsigned nb_tried = 0;
    do {
        if (nb_tried++ >= 4)
            return -1;

        ret = p256_generate_random(sbytes, 32);
        CT_POISON(sbytes, 32);
        if (ret != 0)
            return -1;

        ret = scalar_from_bytes(s, sbytes);
        CT_UNPOISON(&ret, sizeof ret);
    }
    while (ret != 0);

    scalar_mult(x, y, p256_gx, p256_gy, s);

    CT_UNPOISON(x, 32);
    CT_UNPOISON(y, 32);

    return 0;
}











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


int p256_ecdh_shared_secret(uint8_t secret[32], const uint8_t priv[32], const uint8_t peer[64])
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

static void ecdsa_m256_mod_n(uint32_t x[8])
{
    uint32_t t[8];
    uint32_t c = u256_sub(t, x, p256_n.m);
    u256_cmov(x, t, 1 - c);
}

static void ecdsa_m256_from_hash(uint32_t z[8], const uint8_t *h, size_t hlen)
{
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

    ecdsa_m256_mod_n(z);

    m256_prep(z, &p256_n);
}

int p256_ecdsa_sign(uint8_t sig[64], const uint8_t priv[32], const uint8_t *hash, size_t hlen)
{
    CT_POISON(priv, 32);

    int ret;

    uint32_t xr[8], k[8], t3[8], t4[8];

    uint8_t *kb = (uint8_t *) t4;
    ret = scalar_gen_with_pub(kb, k, xr, t3);
    if (ret != 0)
        return P256_RANDOM_FAILED;
    m256_prep(k, &p256_n);

    m256_done(xr, &p256_p);

    ecdsa_m256_mod_n(xr);

    if (u256_diff0(xr) == 0)
        return P256_RANDOM_FAILED;

    u256_to_bytes(sig, xr);

    m256_prep(xr, &p256_n);


    ecdsa_m256_from_hash(t3, hash, hlen);


    ret = scalar_from_bytes(t4, priv);
    CT_UNPOISON(&ret, sizeof ret);
    if (ret != 0)
        return P256_INVALID_PRIVKEY;
    m256_prep(t4, &p256_n);

    m256_inv(k, k, &p256_n);
    m256_mul(t4, xr, t4, &p256_n);
    m256_add(t4, t3, t4, &p256_n);
    m256_mul(t4, k, t4, &p256_n);
    zeroize(k, sizeof k);

    CT_UNPOISON(t4, 32);
    if (u256_diff0(t4) == 0) {
        u256_to_bytes(sig, t4);
        return P256_RANDOM_FAILED;
    }
    m256_to_bytes(sig + 32, t4, &p256_n);

    return P256_SUCCESS;
}

int p256_ecdsa_verify(const uint8_t sig[64], const uint8_t pub[64], const uint8_t *hash, size_t hlen)
{
    int ret;

    uint32_t r[8], s[8];
    ret = scalar_from_bytes(r, sig);
    if (ret != 0)
        return P256_INVALID_SIGNATURE;
    ret = scalar_from_bytes(s, sig + 32);
    if (ret != 0)
        return P256_INVALID_SIGNATURE;


    uint32_t e[8];
    ecdsa_m256_from_hash(e, hash, hlen);

    uint32_t u1[8], u2[8];
    m256_prep(s, &p256_n);
    m256_inv(s, s, &p256_n);
    m256_mul(u1, e, s, &p256_n);
    m256_done(u1, &p256_n);

    u256_cmov(u2, r, 1);
    m256_prep(u2, &p256_n);
    m256_mul(u2, u2, s, &p256_n);
    m256_done(u2, &p256_n);

    uint32_t px[8], py[8];
    ret = point_from_bytes(px, py, pub);
    if (ret != 0)
        return P256_INVALID_PUBKEY;

    scalar_mult(e, s, px, py, u2);

    if (u256_diff0(u1) == 0) {
        u256_cmov(u1, e, 1);
    } else {
        scalar_mult(px, py, p256_gx, p256_gy, u1);

        point_add_or_double_leaky(u1, u2, px, py, e, s);
    }

    m256_done(u1, &p256_p);

    ecdsa_m256_mod_n(u1);

    uint32_t diff = u256_diff(u1, r);
    if (diff == 0)
        return P256_SUCCESS;

    return P256_INVALID_SIGNATURE;
}

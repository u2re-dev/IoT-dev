
/*
 * Implementation of curve P-256 (ECDH and ECDSA)
 * Originally: https://github.com/mpg/p256-m
 *
 * Author: Manuel Pégourié-Gonnard.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef P256_M_H
#define P256_M_H

//
#include <stdint.h>
#include <stddef.h>

//
#define P256_SUCCESS            0
#define P256_RANDOM_FAILED      -1
#define P256_INVALID_PUBKEY     -2
#define P256_INVALID_PRIVKEY    -3
#define P256_INVALID_SIGNATURE  -4

//
#ifdef __cplusplus
extern "C" {
#endif
    extern int p256_generate_random(uint8_t * output, unsigned output_size);
    int p256_gen_keypair(uint8_t priv[32], uint8_t pub[64]);
    int p256_ecdh_shared_secret(uint8_t secret[32], const uint8_t priv[32], const uint8_t pub[64]);
    int p256_ecdsa_sign(uint8_t sig[64], const uint8_t priv[32], const uint8_t *hash, size_t hlen);
    int p256_ecdsa_verify(const uint8_t sig[64], const uint8_t pub[64], const uint8_t *hash, size_t hlen);
#ifdef __cplusplus
}
#endif
#endif

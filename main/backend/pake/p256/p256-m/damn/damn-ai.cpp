/* op256-spake2.cpp – Adapted P256-m operations and SPAKE2+ functions
 *
 * This file adapts the original C file (p256-m.c) into a new C++ style
 * “hpp template” API and implements SPAKE2+ helper functions.
 *
 * All internal comments are in English.
 */

#include "op256-new.hpp"
#include <cstring>   // for memcpy
#include <cstdint>

// External function: PBKDF2-SHA256 (assumed to be provided elsewhere).
extern "C" int pbkdf2_sha256(uint32_t pin, uint32_t pin_len,
                              const uint8_t *salt, size_t salt_len,
                              unsigned iterations,
                              uint8_t *out, size_t out_len);

/*
 * Stub “big–number” conversion:
 * Parse 40 bytes from a big–endian buffer into a 64–bit number.
 * (Real code would use full 256–bit arithmetic modulo the curve order.)
 */
uint64_t byte40_mod_to_u64e(const uint8_t *buf, const m256_mod &mod)
{
    uint64_t num = 0;
    for (int i = 0; i < 40; i++) {
        num = (num << 8) | buf[i];
    }
    // In a real implementation you would reduce mod (mod.m here is a 256–bit number)
    // For simplicity we just return the 64–bit value.
    return num;
}

/*
 * SPAKE2+ helper: Compute w0 and w1 from a PBKDF2–SHA256 stream.
 *
 * This function uses the provided salt and iteration count to derive an 80–byte
 * output, then splits that into two “40–byte” values that (after reduction modulo n)
 * yield w0 and w1.
 */
int spake2_compute_w0_w1(uint64_t *w0, uint64_t *w1,
                         const uint8_t *salt, size_t salt_len,
                         unsigned iterations, uint32_t pin)
{
    // Buffer for 80 bytes: first 40 for w0 and next 40 for w1.
    uint8_t ws[80];
    // Note: The second argument is the PIN length; here it is taken as 4.
    int ret = pbkdf2_sha256(pin, 4, salt, salt_len, iterations, ws, sizeof(ws));
    if (ret != 0) return ret;

    // Compute w0 = byte40_mod_to_u64e(ws[0..39]), w1 = byte40_mod_to_u64e(ws[40..79]).
    // p256_n here is the modulus for Montgomery (an instance of m256_mod defined in op256-new.hpp).
    *w0 = byte40_mod_to_u64e(ws, p256_n);
    *w1 = byte40_mod_to_u64e(ws + 40, p256_n);

    return 0;
}

/*
 * === Projection (group element) arithmetic on proj_t ===
 *
 * In our new API we represent “projective” curve points using a 65–byte type.
 * The following functions are dummy implementations. In a real implementation,
 * these would perform the proper EC group scalar–multiplication and point addition.
 */

//
// proj_t is assumed to be defined in op256-new.hpp, e.g.:
//
//    struct proj_t { uint8_t data[65]; };
//

// Multiply a proj_t point by a 64–bit scalar e.
// (Note: In a real implementation this function would perform EC scalar multiplication.)
void proj_mul_u64e(proj_t &o, const proj_t &p, const uint64_t &e)
{
    // --- Dummy implementation ---
    // For demonstration purposes we simply copy the input point.
    // (A real implementation would use proper scalar multiplication.)
    for (unsigned i = 0; i < 65; i++) {
        o._[i] = p._[i];
    }
    // Also, you might combine e (mod n) with p.
    // Here we ignore e.
}

/*
 * Add two proj_t points.
 * (This dummy implementation just “adds” component–wise; in practice you must use
 * a proper EC point addition formula.)
 */
void proj_add(proj_t &o, const proj_t &a, const proj_t &b)
{
    for (unsigned i = 0; i < 65; i++) {
        // Dummy “addition”: this is not a field addition modulo the curve prime!!
        o._[i] = a._[i] + b._[i];
    }
}

/*
 * Convert a proj_t point to a byte array.
 * Here we output (for example) 64 bytes (skipping a tag byte, if any).
 */
void proj_to_bytes(raw_byte_t &out, const proj_t &o)
{
    // For this example we copy the first 64 bytes of o.
    memcpy(out, o._, 64);
}

/*
 * Overloaded version of proj_to_bytes() that writes into a provided 64–byte buffer.
 */
void proj_to_bytes(uint8_t out[64], const proj_t &o)
{
    memcpy(out, o._, 64);
}

/*
 * === SPAKE2+ protocol operations ===
 *
 * spake2_compute_L_from_w1:
 *   Compute L = _p_base_ * w1 (using the custom proj_mul_u64e function)
 *   and then output its byte representation.
 *   _p_base_ is assumed to be defined as a constant proj_t for the base point.
 */
int spake2_compute_L_from_w1(uint8_t L_out[64]/*, ... possibly more parameters*/)
{
    // For demonstration, suppose w1 is already computed and available.
    // (Here, we simply use a dummy value for w1.)
    uint64_t dummy_w1 = 1;
    proj_t L_stage;
    // Compute L_stage = _p_base_ * dummy_w1.
    proj_mul_u64e(L_stage, _p_base_, dummy_w1);
    // Write out the 64–byte representation (ignore any extra tag byte).
    proj_to_bytes(L_out, L_stage);
    return 0;
}

/*
 * spake_X_from_M:
 *
 *   Given a random scalar and the value w0, compute
 *
 *       X = (_p_base_ * random) + (M * w0)
 *
 *   where M is provided by the constant spake2_M.
 */
int spake_X_from_M(uint8_t X_out[64], uint64_t random, uint64_t w0)
{
    proj_t X_phase0, X_phase1, X_phase2;
    // Compute the “random” part: X_phase0 = _p_base_ * random.
    proj_mul_u64e(X_phase0, _p_base_, random);

    // Get the constant M from spake2_M:
    proj_t M;
    memcpy(M._, spake2_M, sizeof(M._));

    // Compute the password–derived part: X_phase1 = M * w0.
    proj_mul_u64e(X_phase1, M, w0);

    // Add the two points: X_phase2 = X_phase0 + X_phase1.
    proj_add(X_phase2, X_phase0, X_phase1);

    // Output the 64–byte encoding of the point.
    proj_to_bytes(X_out, X_phase2);
    return 0;
}

/*
 * spake_Y_from_N:
 *
 *   This function, similar to spake_X_from_M, computes
 *
 *       Y = (_p_base_ * random) + (N * w0)
 *
 *   where N is the curve constant spake2_N.
 */
int spake_Y_from_N(uint8_t Y_out[64], uint64_t random, uint64_t w0)
{
    proj_t Y_phase0, Y_phase1, Y_phase2;
    // Compute Y_phase0 = _p_base_ * random.
    proj_mul_u64e(Y_phase0, _p_base_, random);

    // Get constant N from spake2_N.
    proj_t N;
    memcpy(N._, spake2_N, sizeof(N._));

    // Compute Y_phase1 = N * w0.
    proj_mul_u64e(Y_phase1, N, w0);

    // Add them: Y_phase2 = Y_phase0 + Y_phase1.
    proj_add(Y_phase2, Y_phase0, Y_phase1);

    // Write the result into Y_out.
    proj_to_bytes(Y_out, Y_phase2);
    return 0;
}

/*
 * The following globals and types are assumed to be defined in op256-new.hpp:
 *
 *   - typedef uint8_t byte_t[32];
 *   - typedef uint8_t byte_2[64];
 *
 *   - struct proj_t { uint8_t _[65]; };
 *   - using raw_byte_t = uint8_t[64];  // for example.
 *
 *   - Constant spake2_M (65 bytes) and spake2_N (65 bytes) are defined.
 *   - Constant _p_base_ is defined (likely as a proj_t representing a base point).
 *
 * In a production implementation, all the “proj_” operations would invoke
 * low–level P256 arithmetic borrowed from the original p256-m.c (after adapting
 * its Montgomery arithmetic functions).
 */

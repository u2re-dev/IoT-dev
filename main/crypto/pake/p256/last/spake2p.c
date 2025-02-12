// pake2plus specific (uint64_t unknown type related)
// also, new data-type `proj_t` (65-byte type based)
// ==================================================
// unknown op type, but needs to implement!
// using proj_t = uint8_t[65]; struct proj_t { uint8_t _[65]; };
// also proj_t may be very related with m256_mod

//
void proj_mul_u64e(proj_t& o, proj_t& const& p, uint64_t const& e);
void proj_add(proj_t& o, proj_t& const& a, proj_t& const& b);
void proj_to_bytes(raw_byte_t& out, proj_t const& o); // needs to look (raw_byte_t is uint8_t[64] or uint8_t[32] was used in reality)

// unknown const (required)
const proj_t _p_base_; // very probably is p256_p or related

// used for compute w0 and w1 (or proj_t mod, I don't know)
void byte40_mod_to_u64e(uint64_t const& e, const uint8_t ws_p[40], const m256_mod *mod);

/* // about byte40_mod_to_u64e usage (from typescript)
 * const ws = await Crypto.pbkdf2(pinWriter.toByteArray(), salt, iterations, CRYPTO_W_SIZE_BYTES * 2);
 * const w0 = mod(bytesToNumberBE(ws.slice(0,  40)), P256_CURVE.n);
 * const w1 = mod(bytesToNumberBE(ws.slice(40, 80)), P256_CURVE.n);
 * return { w0, w1 };
 */

// also under P256_CURVE.n may means p256_n or related



// ==================================== //
// and for spake2plus example (look-at) //
// https://datatracker.ietf.org/doc/html/draft-bar-cfrg-spake2plus-01

// ! it means as proj_t, or sort of (i.e. 65-byte)
static const uint8_t spake2_M[64] = {
    0x28,0x88,0x86,0x6e,0xe2,0x2f,0xf9,0x97,0x7a,0xac,0xce,0xe4,0x46,0x6e,0xe5,0x55,
    0x5b,0xba,0xa9,0x9d,0xdd,0xd7,0x72,0x24,0x42,0x25,0x57,0x79,0x9f,0xf2,0x29,0x99,
    0x93,0x3b,0xb6,0x64,0x4e,0xe1,0x16,0x6e,0xef,0xf3,0x3d,0xdc,0xca,0xab,0xb9,0x95,
    0x5a,0xaf,0xfd,0xd4,0x49,0x97,0x73,0x33,0x33,0x3d,0xd8,0x8f,0xfa,0xa1,0x12,0x2f
};

// ! it means as proj_t, or sort of (i.e. 65-byte)
static const uint8_t spake2_N[64] = {
    0x3d,0xd8,0x8b,0xbb,0xbd,0xd6,0x6c,0xc6,0x63,0x39,0x9c,0xc6,0x62,0x29,0x93,0x37,
    0x7b,0xb0,0x04,0x4d,0xd9,0x99,0x97,0x7f,0xf3,0x38,0x8c,0xc3,0x37,0x77,0x70,0x07,
    0x71,0x19,0x9c,0xc6,0x62,0x29,0x9d,0xd7,0x70,0x01,0x14,0x4d,0xd4,0x49,0x9a,0xa2,
    0x24,0x4b,0xb4,0x4f,0xf9,0x98,0x8b,0xba,0xaa,0xa1,0x12,0x29,0x92,0x2b,0xb4,0x49
};

//
int spake2_compute_w0_w1(uint64_t *w0, uint64_t *w1, const uint8_t *salt, size_t salt_len, unsigned iterations, uint32_t pin)
{
    uint8_t ws[80];
    int ret = pbkdf2_sha256(pin, 4, salt, salt_len, iterations, ws, 80);
    if (ret != 0) return ret;

    //
    *w0 = byte40_mod_to_u64e(slice(ws, 0 , 40), p256_n);
    *w1 = byte40_mod_to_u64e(slice(ws, 40, 80), p256_n);

    //
    return 0;
}

//
int spake2_compute_L_from_w1(uint8_t L_out[64]/*, const uint8_t *salt, size_t salt_len, unsigned iterations, uint32_t pin*/)
{
    uint64_t w0, w1; // допустим они уже есть...
    //int ret = spake2_compute_w0w1_big(&w0, &w1, salt, salt_len, iterations, pin);
    //if (ret != 0) return ret;

    // необычная операция к имплементации
    // и какая-то константа _p_base_
    proj_mul_u64e(L_stage, _p_base_, w1);
    proj_to_bytes(L_out);

    //
    return 0;
}

//
int spake_X_from_M(uint8_t X_out[64], uint64_t random, uint64_t w0)
{
    proj_mul_u64e(X_phase0, _p_base_, random); // необычная операция к имплементации
    proj_mul_u64e(X_phase1, M, w0); // необычная операция к имплементации
    proj_add(X_phase2, X_phase0, X_phase1); // тут вроде по классике
    proj_to_bytes(X_out, X_phase2);
    return 0;
}

//
int spake_Y_from_N(uint8_t Y_out[64], uint64_t random, uint64_t w0)
{   // я могу ошибаться, но этом либо m256_, либо point_
    proj_mul_u64e(Y_phase0, _p_base_, random); // необычная операция к имплементации
    proj_mul_u64e(Y_phase1, N, w0); // необычная операция к имплементации
    proj_add(Y_phase2, N_phase0, N_phase1); // тут вроде по классике
    proj_to_bytes(Y_out, Y_phase2);
    return 0;
}




//
extern int pbkdf2_sha256(const uint8_t *pass, size_t passlen, const uint8_t *salt, size_t saltlen, unsigned iterations, uint8_t *out, size_t outlen);

//
int spake2_client_compute_key_big(uint8_t key_out[32], uint64_t w0, const uint8_t Y_peer[64])
{
    return 0;
}

//
int spake2_server_compute_key_big(uint8_t key_out[32], uint64_t w0, const uint8_t X_peer[64])
{
    return 0;
}

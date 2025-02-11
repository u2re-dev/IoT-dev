
// !BROKEN CODE BEGIN! //

// ! it means as proj_t, or sort of (i.e. 65-byte)
static const uint8_t spake2_M[65] = {
    0x02,0x28,0x88,0x86,0x6e,0xe2,0x2f,0xf9,0x97,0x7a,0xac,0xce,0xe4,0x46,0x6e,0xe5,
    0x55,0x5b,0xba,0xa9,0x9d,0xdd,0xd7,0x72,0x24,0x42,0x25,0x57,0x79,0x9f,0xf2,0x29,
    0x99,0x93,0x3b,0xb6,0x64,0x4e,0xe1,0x16,0x6e,0xef,0xf3,0x3d,0xdc,0xca,0xab,0xb9,
    0x95,0x5a,0xaf,0xfd,0xd4,0x49,0x97,0x73,0x33,0x33,0x3d,0xd8,0x8f,0xfa,0xa1,0x12,
    0x2f
};

// ! it means as proj_t, or sort of (i.e. 65-byte)
static const uint8_t spake2_N[65] = {
    0x03,0x3d,0xd8,0x8b,0xbb,0xbd,0xd6,0x6c,0xc6,0x63,0x39,0x9c,0xc6,0x62,0x29,0x93,
    0x37,0x7b,0xb0,0x04,0x4d,0xd9,0x99,0x97,0x7f,0xf3,0x38,0x8c,0xc3,0x37,0x77,0x70,
    0x07,0x71,0x19,0x9c,0xc6,0x62,0x29,0x9d,0xd7,0x70,0x01,0x14,0x4d,0xd4,0x49,0x9a,
    0xa2,0x24,0x4b,0xb4,0x4f,0xf9,0x98,0x8b,0xba,0xaa,0xa1,0x12,0x29,0x92,0x2b,0xb4,
    0x49
};

//
extern int pbkdf2_sha256(const uint8_t *pass, size_t passlen, const uint8_t *salt, size_t saltlen, unsigned iterations, uint8_t *out, size_t outlen);

//
int spake2_compute_w0w1_big(uint64_t *w0, uint64_t *w1, const uint8_t *salt, size_t salt_len, unsigned iterations, uint32_t pin)
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
int spake2_compute_w0L_big(uint8_t L_out[64], const uint8_t *salt, size_t salt_len, unsigned iterations, uint32_t pin)
{
    uint64_t w0, w1;
    int ret = spake2_compute_w0w1_big(&w0, &w1, salt, salt_len, iterations, pin);
    if (ret != 0) return ret;

    // необычная операция к имплементации
    // и какая-то константа _p_base_
    m256_mul_u64e(L, _p_base_, w1);

    //
    return 0;
}



//
int spake2_client_round1_big(uint8_t X_out[64], uint64_t random, uint64_t w0)
{
    proj_mul_u64e(X_phase0, _p_base_, random); // необычная операция к имплементации
    proj_mul_u64e(X_phase1, M, w0); // необычная операция к имплементации
    proj_add(X_phase2, X_phase0, X_phase1); // тут вроде по классике
    proj_to_bytes(X_out, X_phase2);
    return 0;
}

//
int spake2_server_round1_big(uint8_t Y_out[64], uint64_t random, uint64_t w0)
{   // я могу ошибаться, но этом либо m256_, либо point_
    proj_mul_u64e(Y_phase0, _p_base_, random); // необычная операция к имплементации
    proj_mul_u64e(Y_phase1, N, w0); // необычная операция к имплементации
    proj_add(Y_phase2, N_phase0, N_phase1); // тут вроде по классике
    proj_to_bytes(Y_out, Y_phase2);
    return 0;
}



//
int spake2_client_compute_key_big(uint8_t key_out[32], uint64_t w0, const uint8_t Y_peer[64])
{
    /* Вычисляем -N, то есть меняем знак y-координаты */
    /* Вычисляем скорректированную точку: Y_adj = Y + (-N) */
    /* Вычисляем общий секрет: shared = w0 • (Y - N) */
    return 0;
}

//
int spake2_server_compute_key_big(uint8_t key_out[32], uint64_t w0, const uint8_t X_peer[64])
{
    /* Получаем -M: инвертируем знак y-координаты */
    /* Вычисляем X_adj = X + (-M) */
    /* Вычисляем общий секрет: shared = w0 • (X - M) */
    return 0;
}

// !BROKEN CODE END! //

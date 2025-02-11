
// !BROKEN CODE BEGIN! //





//
extern int pbkdf2_sha256(const uint8_t *pass, size_t passlen, const uint8_t *salt, size_t saltlen, unsigned iterations, uint8_t *out, size_t outlen);





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

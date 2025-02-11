#include <iostream>
#include <vector>
#include <cstring>

//
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/rand.h>

//
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN = CRYPTO_W_SIZE_BYTES * 2;

//
void uint32_to_le_bytes(uint32_t pin, unsigned char out[4]) {
    out[0] = static_cast<unsigned char>( pin        & 0xFF );
    out[1] = static_cast<unsigned char>((pin >> 8)  & 0xFF );
    out[2] = static_cast<unsigned char>((pin >> 16) & 0xFF );
    out[3] = static_cast<unsigned char>((pin >> 24) & 0xFF );
}

//
int computeW0W1(uint32_t pin, const std::vector<unsigned char>& salt, int iterations,
                BIGNUM** w0, BIGNUM** w1)
{
    unsigned char output[PBKDF2_OUTLEN] = {0};
    unsigned char pin_bytes[4] = {0};
    uint32_to_le_bytes(pin, pin_bytes);

    if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(pin_bytes), 4,
                          salt.data(), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), PBKDF2_OUTLEN, output) != 1) {
        std::cerr << "PBKDF2_HMAC failed\n";
        return -1;
    }

    *w0 = BN_new();
    *w1 = BN_new();
    if (!*w0 || !*w1) {
        std::cerr << "BN_new failed\n";
        return -1;
    }
    // Первые 40 байт → w0, следующие → w1.
    BN_bin2bn(output, CRYPTO_W_SIZE_BYTES, *w0);
    BN_bin2bn(output + CRYPTO_W_SIZE_BYTES, CRYPTO_W_SIZE_BYTES, *w1);

    // Порядок n для P-256:
    BIGNUM* order = nullptr;
    BN_hex2bn(&order, "FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551");
    if (!order) {
        std::cerr << "Ошибка создания order\n";
        BN_free(*w0); BN_free(*w1);
        return -1;
    }
    BN_CTX* ctx = BN_CTX_new();
    if (!ctx) {
        std::cerr << "BN_CTX_new failed\n";
        BN_free(*w0); BN_free(*w1); BN_free(order);
        return -1;
    }
    BN_mod(*w0, *w0, order, ctx);
    BN_mod(*w1, *w1, order, ctx);

    //
    BN_free(order);
    BN_CTX_free(ctx);
    return 0;
}

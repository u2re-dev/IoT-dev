#pragma once
#include <aes/esp_aes.h>

//
#define AES128 1
#define AES_BLOCKLEN 16

struct AES_ctx
{
    esp_aes_context RoundKey;
    uint8_t Iv[AES_BLOCKLEN];
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key)
{
    esp_aes_init(&ctx->RoundKey);
    esp_aes_setkey(&ctx->RoundKey, key, 128);
};

void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv)
{
    esp_aes_init(&ctx->RoundKey);
    esp_aes_setkey(&ctx->RoundKey, key, 128);
    memcpy (ctx->Iv, iv, AES_BLOCKLEN);
};

void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv)
{
    memcpy (ctx->Iv, iv, AES_BLOCKLEN);
};

void AES_CBC_encrypt_buffer(struct AES_ctx *ctx, uint8_t* buf, size_t length)
{
    esp_aes_crypt_cbc(&ctx->RoundKey, ESP_AES_ENCRYPT, length, ctx->Iv, buf, buf);
};

void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, size_t length)
{
    esp_aes_crypt_cbc(&ctx->RoundKey, ESP_AES_DECRYPT, length, ctx->Iv, buf, buf);
};

void AES_ECB_encrypt(struct AES_ctx* ctx, uint8_t* buf)
{
    esp_aes_crypt_ecb (&ctx->RoundKey, ESP_AES_ENCRYPT, buf, buf);
};

void AES_ECB_decrypt(struct AES_ctx* ctx, uint8_t* buf)
{
    esp_aes_crypt_ecb (&ctx->RoundKey, ESP_AES_DECRYPT, buf, buf);
};

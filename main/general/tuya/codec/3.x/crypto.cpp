#include "../libtuya.hpp"

//
#define AES128 1
#define AES_BLOCKLEN 16
#ifdef USE_ESP32_NATIVE
#include <aes/esp_aes.h>

//
namespace tc {

    //
    block_t encryptDataECB(block_t const& key,  block_t const& data) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);
        block_t output = data; esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, &output, &output);
        esp_aes_free(&ctx);
        return output;
    }

    //
    block_t decryptDataECB(block_t const& key,  block_t const& data) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);
        block_t output = data; esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, &output, &output);
        esp_aes_free(&ctx);
        return output;
    }



    //
    bytespan_t encryptDataECB(block_t const& key,  bytespan_t const& data, const bool usePadding) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);

        // add post-padding
        const auto padded = ((data->size() + 16 /*- 1*/) >> 4) << 4;
        auto output = usePadding ? bytespan_t(data->data(), padded) : data;
        if (usePadding) { for (uint I=data->size();I<padded;I++) { output[I] = (padded - data->size()); }}

        //
        for (uint I=0;I<output->size();I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, output->data()+I, output->data()+I);
        }

        // add padding value
        esp_aes_free(&ctx);
        return output;
    }

    //
    bytespan_t decryptDataECB(block_t const& key,  bytespan_t const& data) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);

        //
        size_t length = data->size();
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, data->data()+I, data->data()+I);
        }

        // re-correction of length (if possible)
        const auto pad = data[length-1];
        if (pad <= 16 && pad > 0 && length > 16) { length -= pad; };
        esp_aes_free(&ctx);

        //
        return bytespan_t(data->data(), length);
    }
};

#else
#include <mbedtls/aes.h>

//
namespace tc {

    //
    block_t encryptDataECB(block_t const& key,  tc::block_t const& data) {
        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_enc(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);
        block_t output = data; mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, reinterpret_cast<uint8_t const*>(&output), reinterpret_cast<uint8_t*>(&output));
        mbedtls_aes_free(&ctx);
        return output;
    }

    //
    block_t decryptDataECB(block_t const& key,  block_t const& data) {
        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_dec(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);
        block_t output = data; mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, reinterpret_cast<uint8_t const*>(&output), reinterpret_cast<uint8_t*>(&output));
        mbedtls_aes_free(&ctx);
        return output;
    }



    //
    bytespan_t encryptDataECB(block_t const& key,  bytespan_t const& data, const bool usePadding) {
        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_enc(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);

        // add post-padding
        const auto padded = ((data->size() + 16 /*- 1*/) >> 4) << 4;
        auto output = usePadding ? bytespan_t(data->data(), padded) : data;
        if (usePadding) { for (uint I=data->size();I<padded;I++) { output[I] = (padded - data->size()); }}

        //
        for (uint I=0;I<output->size();I+=16) {
            mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, output->data()+I, output->data()+I);
        }

        // add padding value
        mbedtls_aes_free(&ctx);
        return output;
    }

    //
    bytespan_t decryptDataECB(block_t const& key,  bytespan_t const& data) {
        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_dec(&ctx, reinterpret_cast<uint8_t const*>(&key), 128);

        //
        size_t length = data->size();
        for (uint I=0;I<length;I+=16) {
            mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, data->data()+I, data->data()+I);
        }

        // re-correction of length (if possible)
        const auto pad = data[length-1];
        if (pad <= 16 && pad > 0 && length > 16) { length -= pad; };
        mbedtls_aes_free(&ctx);

        //
        return bytespan_t(data->data(), length);
    }
};

//
#endif

#include "../libtuya.hpp"

//
#define AES128 1
#define AES_BLOCKLEN 16
#include <aes/esp_aes.h>

//
namespace tc {

    //
    bigint_t encryptDataECB(bigint_t const& key,  bigint_t const& data) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);
        bigint_t output = data; esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, &output, &output);
        esp_aes_free(&ctx);
        return output;
    }

    //
    bigint_t decryptDataECB(bigint_t const& key,  bigint_t const& data) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);
        bigint_t output = data; esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, &output, &output);
        esp_aes_free(&ctx);
        return output;
    }



    //
    bytespan_t encryptDataECB(bigint_t const& key,  bytespan_t const& data, const bool usePadding) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        // add post-padding
        const auto padded = ((length + 16 /*- 1*/) >> 4) << 4; auto output = bytespan_t(data->data(), padded);
        if (usePadding) { for (uint I=length;I<padded;I++) { output[I] = (padded - length); }}

        //
        for (uint I=0;I<(usePadding ? padded : length);I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, output->data()+I, output->data()+I);
        }

        // add padding value
        if (usePadding) { length = padded; };
        esp_aes_free(&ctx);

        //
        return output;
    }

    //
    bytespan_t decryptDataECB(bigint_t const& key,  bytespan_t const& data) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

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

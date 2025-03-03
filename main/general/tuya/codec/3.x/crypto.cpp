#include "../libtuya.hpp"

//
#define AES128 1
#define AES_BLOCKLEN 16
#include <aes/esp_aes.h>

//
namespace tc {

    //
    uint8_t* encryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output, const bool usePadding) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        // prepare output to encrypt
        if (output) memcpy(output, data, length);
        output = output ? output : data;

        // add post-padding
        const auto padded = ((length + 16 /*- 1*/) >> 4) << 4;
        if (usePadding) { for (uint I=length;I<padded;I++) { output[I] = (padded - length); }}

        //
        for (uint I=0;I<(usePadding ? padded : length);I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, output+I, output+I);
        }

        // add padding value
        if (usePadding) { length = padded; };
        esp_aes_free(&ctx);

        //
        return output;
    }

    //
    uint8_t* decryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output) {
        esp_aes_context ctx;
        esp_aes_init(&ctx);
        esp_aes_setkey(&ctx, key, 128);

        //
        output = output ? output : data;
        for (uint I=0;I<length;I+=16) {
            esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, data+I, output+I);
        }

        // re-correction of length (if possible)
        const auto pad = data[length-1];
        if (pad <= 16 && pad > 0 && length > 16) { length -= pad; };
        esp_aes_free(&ctx);

        //
        return output;
    }
}

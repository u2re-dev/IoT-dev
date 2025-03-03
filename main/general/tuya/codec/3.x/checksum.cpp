#include "../libtuya.hpp"

//
#ifdef USE_ESP32_NATIVE
//#include <esp32s3/rom/crc.h>
#else
#include <mbedtls/md.h>
#endif

//
namespace tc {

    //
    uint32_t crc32(uint8_t const* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc >>= 1;
                }
            }
        }
        return (~crc);
    }

    // TODO? needs to merge into payload part?
    bytespan_t checksumTuyaCode(bytes_t& code, block_t const& HMAC) { bytespan_t span = code; return checksumTuyaCode(span, HMAC); };
    bytespan_t checksumTuyaCode(bytespan_t& code, block_t const& HMAC) {
        const auto headerLen   = 16;
        const auto payloadSize = bswap32(*reinterpret_cast<uint32_t const*>(code->data()+12));
        const auto Is          = (HMAC ? 32 : 4);
        const auto length      = payloadSize - (Is + 4);
        for (auto i=0;i<Is;i++) { code[headerLen + length + i] = 0; }

        //
        if (HMAC) {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, reinterpret_cast<uint8_t const*>(&HMAC), 16);
            mbedtls_md_hmac_update(&ctx, code->data() , headerLen + length); // header + payload
            mbedtls_md_hmac_finish(&ctx, code->data() + headerLen + length); // write after payload
            mbedtls_md_free(&ctx);
        } else {
#ifdef USE_ESP32_NATIVE
            *reinterpret_cast<uint32_t*>(code->data() + headerLen + length) = crc32_be(0, code, length + header_len);
#else
            *reinterpret_cast<uint32_t*>(code->data() + headerLen + length) = bswap32(crc32(code->data(), length + headerLen));
#endif
        }

        //
        return bytespan_t(code->data(), payloadSize + headerLen);
    }
};

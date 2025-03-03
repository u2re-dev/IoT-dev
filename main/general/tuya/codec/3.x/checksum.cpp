
#include "../libtuya.hpp"

//
#include <esp32s3/rom/crc.h>
#include <mbedtls/md.h>

//
namespace tc {

    // TODO? needs to merge into payload part?
    bytespan_t checksumTuyaCode(bytespan_t const& code, bigint_t const& HMAC = 0) {
        const auto header_len  = 16;
        const auto payloadSize = bswap32(*reinterpret_cast<uint32_t const*>(code->data()+12));
        const auto payload     = code->data() + header_len;
        const auto length      = payloadSize - ((HMAC ? 32 : 4) + 4);
        for (uint i=0;i<(HMAC ? 32 : 4);i++) { payload[length + i] = 0; }

        //
        if (HMAC) {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, reinterpret_cast<uint32_t const*>(&HMAC), 16);
            mbedtls_md_hmac_update(&ctx, code->data(), length + header_len); // header + payload
            mbedtls_md_hmac_finish(&ctx, payload + length); // write after payload
            mbedtls_md_free(&ctx);
        } else {
            *(uint32_t*)(payload + length) = crc32_be(0, code, length + header_len);
        }

        //
        return bytespan_t(code, payloadSize + header_len);
    }
};

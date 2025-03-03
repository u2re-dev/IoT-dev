//#include "../../crypto/soc.hpp"
#include "../libtuya.hpp"

//
namespace tc {
    // HMAC i.e. hmac_key
    size_t prepareTuyaCode(size_t& length, TuyaCmd const& cmdDesc, uint8_t* output) {
        // write header
        *(uint32_t*)(output+0) = bswap32(0x000055AA);

        // encode as big-endian
        const uint32_t header_len = 16;
        const auto payloadSize  = computePayloadSize(length, cmdDesc.HMAC ? true : false);
        *(uint32_t*)(output+4)  = bswap32(cmdDesc.SEQ_NO);
        *(uint32_t*)(output+8)  = bswap32(cmdDesc.CMD_ID);
        *(uint32_t*)(output+12) = bswap32(payloadSize);

        //
        const auto payload = output + header_len;
        for (uint i=0;i<(length + (cmdDesc.HMAC ? 32 : 4));i++) {
            *(payload + i) = 0;
        }

        // write suffix
        *(uint32_t*)(payload + length + (cmdDesc.HMAC ? 32 : 4)) = bswap32(0x0000AA55);

        //
        return payloadSize + header_len;
    }

    // TODO? needs to merge into payload part?
    size_t checksumTuyaCode(uint8_t* code, uint8_t* HMAC) {
        const uint32_t header_len = 16;
        const auto payloadSize = bswap32(*(uint32_t const*)(code+12));
        const auto payload = code + header_len;
        const auto length = payloadSize - ((HMAC ? 32 : 4) + 4);

        //
        for (uint i=0;i<(HMAC ? 32 : 4);i++) {
            *(payload + length + i) = 0;
        }

        //
        if (HMAC) {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
            mbedtls_md_hmac_starts(&ctx, (const unsigned char *) HMAC, 16);
            mbedtls_md_hmac_update(&ctx, (const unsigned char *) code, length + header_len); // header + payload
            mbedtls_md_hmac_finish(&ctx, payload + length); // write after payload
            mbedtls_md_free(&ctx);
        } else {
            *(uint32_t*)(payload + length) = crc32_be(0, code, length + header_len);
        }

        //
        return payloadSize + header_len;
    }

    // HMAC i.e. hmac_key
    size_t encodeTuyaCode(uint8_t* encrypted_data, size_t& length, TuyaCmd const& cmdDesc, uint8_t* output) {
        prepareTuyaCode(length, cmdDesc, output);
        memcpy(output + 16, encrypted_data, length);
        return checksumTuyaCode(output, cmdDesc.HMAC);
    }
};

#pragma once

//
#include <string>
#include <mbedtls/hkdf.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha256.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ecp.h>
#include <mbedtls/ecjpake.h>
#include "./raii/misc.hpp"

//
#include "../std/types.hpp"

//
namespace crypto {
    //
    inline bytespan_t pbkdf2(uint8_t const* input, size_t const& plen, const bigint_t& salt, uint32_t iterations, size_t outputLength = 80) {
        auto output = make_bytes(outputLength);
        auto inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

        //
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        checkMbedtlsError(mbedtls_md_setup(&ctx, inf, 1), "pbkdf2 setup failed");
        checkMbedtlsError(mbedtls_pkcs5_pbkdf2_hmac(&ctx, 
            input, plen, 
            reinterpret_cast<uint8_t const*>(&salt), sizeof(salt), 
            iterations, 
            output->size(), output->data()
        ), "pbkdf2 failed");
        mbedtls_md_free(&ctx);
        return output;
    };

    //
    inline bytespan_t hkdf_len(auto const& ikm, const bytespan_t& info, size_t const& length = 48) {
        auto out = make_bytes(length);
        auto inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);;
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx); mbedtls_md_setup(&ctx, inf, 1);
        checkMbedtlsError(mbedtls_hkdf(inf, 
            nullptr, 0, 
            reinterpret_cast<uint8_t const*>(&ikm), sizeof(ikm), 
            info->data(), info->size(), 
            out->data(), out->size()
        ), "HKDF failed");
        mbedtls_md_free(&ctx);
        return out;
    };

    //
    inline bigint_t hkdf(auto const& ikm, const bytespan_t& info) {
        bigint_t out = bigint_t(0);
        mbedtls_md_context_t ctx; const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md_init(&ctx); mbedtls_md_setup(&ctx, inf, 1);
        checkMbedtlsError(mbedtls_hkdf(inf, 
            nullptr, 0, 
            reinterpret_cast<uint8_t const*>(&ikm), sizeof(ikm), 
            info->data(), info->size(), 
            reinterpret_cast<uint8_t*>(&out), sizeof(out)
        ), "HKDF failed");
        mbedtls_md_free(&ctx);
        return out;
    };

    //
    inline bigint_t hmac(const auto& key, const bytespan_t& data) {
        bigint_t out = bigint_t(0);
        mbedtls_md_context_t ctx; const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md_init(&ctx); mbedtls_md_setup(&ctx, info, 1);
        checkMbedtlsError(mbedtls_md_hmac_starts(&ctx, reinterpret_cast<uint8_t const*>(&key),  sizeof(key)), "HMAC Failed (key)");
        checkMbedtlsError(mbedtls_md_hmac_update(&ctx, data->data(), data->size()), "HMAC Failed (data)");
        checkMbedtlsError(mbedtls_md_hmac_finish(&ctx, reinterpret_cast<uint8_t*>(&out)), "HMAC Failed (finish)");
        mbedtls_md_free(&ctx);
        return out;
    };

    //
    inline bigint_t hash(const bytespan_t& data) {
        bigint_t out = bigint_t(0);
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts_ret(&ctx, 0);
        checkMbedtlsError(mbedtls_sha256_update_ret(&ctx, data->data(), data->size()), "Hash Update Failed");
        checkMbedtlsError(mbedtls_sha256_finish_ret(&ctx, reinterpret_cast<uint8_t*>(&out)), "Failed to compute Hash");
        return out;
    };
}

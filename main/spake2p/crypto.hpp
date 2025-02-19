#pragma once

//
#include <utility>
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
    inline bytes_t pbkdf2(uint8_t const* input, size_t const& plen, const bigint_t& salt, uint32_t iterations, size_t outputLength = 80) {
        bytes_t output = make_bytes(outputLength);
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);

        //
        const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        checkMbedtlsError(mbedtls_md_setup(&ctx, inf, 1), "pbkdf2 setup failed");
        checkMbedtlsError(mbedtls_pkcs5_pbkdf2_hmac(&ctx, 
            input, 
            plen, 
            (uint8_t*)&salt, 
            sizeof(salt), 
            iterations, 
            output->size(), output->data()
        ), "pbkdf2 failed");
        mbedtls_md_free(&ctx);

        //
        return std::move(output);
    };

    //
    inline bigint_t hkdf(auto const& ikm, const bytes_t& info) {
        bigint_t out = bigint_t(0);

        //
        mbedtls_md_context_t ctx; const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md_init(&ctx); mbedtls_md_setup(&ctx, inf, 1);
        checkMbedtlsError(mbedtls_hkdf(inf, 
            nullptr, 0, 
            (uint8_t*)&ikm, sizeof(ikm), 
            info->data(), info->size(), 
            (uint8_t*)&out, sizeof(out)
        ), "HKDF failed");
        mbedtls_md_free(&ctx);
        return std::move(out);
    };

    //
    inline bigint_t hmac(const auto& key, const bytes_t& data) {
        bigint_t out = bigint_t(0);

        //
        mbedtls_md_context_t ctx; const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md_init(&ctx); mbedtls_md_setup(&ctx, info, 1);
        checkMbedtlsError(mbedtls_md_hmac_starts(&ctx, (uint8_t const*)&key,  sizeof(key)), "HMAC Failed (key)");
        checkMbedtlsError(mbedtls_md_hmac_update(&ctx, (uint8_t const*)data->data(), data->size()), "HMAC Failed (data)");
        checkMbedtlsError(mbedtls_md_hmac_finish(&ctx, (uint8_t*)&out), "HMAC Failed (finish)");
        mbedtls_md_free(&ctx);
        return std::move(out);
    };

    //
    inline bigint_t hash(const bytes_t& data) {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts_ret(&ctx, 0);
        mbedtls_sha256_update_ret(&ctx, data->data(), data->size());

        //
        bigint_t out = bigint_t(0);
        checkMbedtlsError(mbedtls_sha256_finish_ret(&ctx, (uint8_t*)&out), "Failed to compute Hash");
        return std::move(out);
    };
}

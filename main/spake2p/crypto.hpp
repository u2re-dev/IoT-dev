#pragma once

//
#include <mbedtls/hkdf.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha256.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ecp.h>
#include <mbedtls/ecjpake.h>
#include "../std/types.hpp"
#include "./raii/misc.hpp"


//
namespace crypto {
    bytes_t pbkdf2(const bytes_t& input, const bytes_t& salt, uint32_t iterations, size_t outputLength) {
        bytes_t output = bytes_t(outputLength);

        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);

        //
        const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        checkMbedtlsError(mbedtls_md_setup(&ctx, inf, 1), "pbkdf2 setup failed");
        checkMbedtlsError(mbedtls_pkcs5_pbkdf2_hmac(&ctx, input.data(), input.size(), salt.data(), salt.size(), iterations, outputLength, output.data()), "pbkdf2 failed");
        mbedtls_md_free(&ctx);

        //
        return output;
    };

    //
    bytes_t hkdf(const bytes_t& ikm, const bytes_t& salt, const bytes_t& info, size_t L) {
        bytes_t output = bytes_t(L);

        mbedtls_md_context_t ctx;
        const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

        mbedtls_md_init(&ctx);
        mbedtls_hkdf(inf, salt.data(), salt.size(), ikm.data(), ikm.size(), info.data(), info.size(), output.data(), L);

        mbedtls_md_free(&ctx);
        return output;
    };

    //
    bytes_t hmac(const bytes_t& key, const bytes_t& data) {
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);

        //
        bytes_t out = bytes_t(32);
        const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md_setup(&ctx, info, 1);
        mbedtls_md_hmac_starts(&ctx,  key.data(),  key.size());
        mbedtls_md_hmac_update(&ctx, data.data(), data.size());
        mbedtls_md_hmac_finish(&ctx,  out.data());
        mbedtls_md_free(&ctx);
        return out;
    };

    //
    bytes_t hash(const bytes_t& data) {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts_ret(&ctx, 0);
        mbedtls_sha256_update_ret(&ctx, data.data(), data.size());

        //
        bytes_t out = bytes_t(32);
        mbedtls_sha256_finish_ret(&ctx, out.data());
        return out;
    };
}

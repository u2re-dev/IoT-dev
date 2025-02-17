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
    bytes_t pbkdf2(const bytes_t& input, const bytes_t& salt, uint32_t iterations, size_t outputLength = 32) {
        bytes_t output = make_bytes(outputLength);
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);

        //
        const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        checkMbedtlsError(mbedtls_md_setup(&ctx, inf, 1), "pbkdf2 setup failed");
        checkMbedtlsError(mbedtls_pkcs5_pbkdf2_hmac(&ctx, input->data(), input->size(), salt->data(), salt->size(), iterations, output->size(), output->data()), "pbkdf2 failed");
        mbedtls_md_free(&ctx);

        //
        return output;
    };

    //
    bigint_t hkdf(auto const* ikm, size_t const& ilen, const bytes_t& salt, const bytes_t& info) {
        bigint_t output = 0;

        mbedtls_md_context_t ctx;
        const mbedtls_md_info_t *inf = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

        mbedtls_md_init(&ctx);
        mbedtls_hkdf(inf, salt->data(), salt->size(), (uint8_t*)ikm, ilen, info->data(), info->size(), (uint8_t*)&output, sizeof(bigint_t));

        mbedtls_md_free(&ctx);
        return output;
    };

    //
    bigint_t hmac(const auto& key, const bytes_t& data) {
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);

        //
        bigint_t out = 0;
        const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md_setup(&ctx, info, 1);
        mbedtls_md_hmac_starts(&ctx, (uint8_t const*)&key,  sizeof(key));
        mbedtls_md_hmac_update(&ctx, (uint8_t const*)data->data(), data->size());
        mbedtls_md_hmac_finish(&ctx, (uint8_t*)&out);
        mbedtls_md_free(&ctx);
        return out;
    };

    //
    bigint_t hash(const bytes_t& data) {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts_ret(&ctx, 0);
        mbedtls_sha256_update_ret(&ctx, data->data(), data->size());

        //
        bigint_t out = 0;
        mbedtls_sha256_finish_ret(&ctx, (uint8_t*)&out);
        return out;
    };
}

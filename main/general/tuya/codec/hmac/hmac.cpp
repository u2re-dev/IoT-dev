#include "../libtuya.hpp"

//
#include <mbedtls/md.h>

//
namespace tc {

    // for protocol 3.4, remote_nonce is encrypted
    bytespan_t encode_remote_hmac(bigint_t const& localkey, bigint_t const& remote_nonce) {
        bytespan_t remote_hmac = make_bytes(48);
        auto denonce = decryptDataECB(localkey, remote_nonce);

        //
        mbedtls_md_context_t ctx;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
        mbedtls_md_hmac_starts(&ctx, reinterpret_cast<uint8_t const*>(&localkey), sizeof(localkey));
        mbedtls_md_hmac_update(&ctx, reinterpret_cast<uint8_t const*>(&denonce), sizeof(denonce));
        mbedtls_md_hmac_finish(&ctx, remote_hmac);
        mbedtls_md_free(&ctx);

        //
        return encryptDataECB(localkey, remote_hmac, false);
    }

    // TODO: support for GCM (data-stream)
    bigint_t encode_hmac_key(bigint_t const& original_key, bigint_t const& remote_nonce) {
        bigint_t local_key = local_nonce ^ remote_nonce;
        return encryptDataECB(original_key, local_key);
    }

};

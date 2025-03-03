#include "../session.hpp"

//
namespace th {
    // MSG 0x3u at beginning
    bigint_t TuyaSession::encodeLocalNonce() {
        return (local_nonce_crypt = tc::encryptDataECB(*(bigint_t const*)(tuya_local_key.c_str()), *(bigint_t const*)tc::local_nonce));
    }

    // when `code == 0x4`
    bytespan_t TuyaSession::sharedNonce(bigint_t const& remote_nonce) {
        hmac = make_bytes(16 + 16 + 12); hmac_key = bytespan_t(hmac_payload->data(), 12);
        return tc::encode_remote_hmac(hmac_key, remote_nonce);
    }

    // after sending shared-nonce
    bigint_t TuyaSession::resolveKey(bigint_t const& remote_nonce) {
        linked = true;
        return tc::encode_hmac_key(*(bigint_t const*)(tuya_local_key.c_str()), remote_nonce);
    }
}

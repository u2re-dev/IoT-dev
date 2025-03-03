#include "../session.hpp"

//
namespace th {
    // MSG 0x3u at beginning
    bytespan_t TuyaSession::encodeLocalNonce() {
        local_nonce_crypt = tc::encryptDataECB(
            *(tc::block_t const*)(tuya_local_key.c_str()),
            *(tc::block_t const*)tc::local_nonce
        );
        return encodeMessage(0x3, bytespan_t(reinterpret_cast<uint8_t const*>(&local_nonce_crypt), 16));
    }

    // when `code == 0x4`
    bytespan_t TuyaSession::sharedNonce(tc::block_t const& remote_nonce) {
        return tc::encode_remote_hmac(hmac_key, remote_nonce);
    }

    // after sending shared-nonce
    tc::block_t TuyaSession::resolveKey(tc::block_t const& remote_nonce) {
        linked = true;
        return tc::encode_hmac_key(*(tc::block_t const*)(tuya_local_key.c_str()), remote_nonce);
    }
}

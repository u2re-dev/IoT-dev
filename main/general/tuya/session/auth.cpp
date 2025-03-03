#include "../session.hpp"

//
namespace th {
    // MSG 0x3u at beginning
    uint8_t* TuyaSession::encodeLocalNonce() {
        size_t keyLen = 16; // say hello with local_nonce with local_key encryption and checksum
        return tc::encryptDataECB((uint8_t *)tuya_local_key.c_str(), (uint8_t *)tc::local_nonce, keyLen, tmp, false);
    }

    // when `code == 0x4`
    uint8_t* TuyaSession::sharedNonce(uint8_t *remote_nonce) {
        size_t hmac_len = 48; // make linked with local remote hmac
        hmac = (uint8_t *)calloc(1, hmac_len); hmac_key = hmac_payload + 12;
        tc::encode_remote_hmac(hmac_key, remote_nonce, hmac);
        return hmac;
    }

    // after sending shared-nonce
    void TuyaSession::resolveKey(uint8_t *remote_nonce) {
        tc::encode_hmac_key((uint8_t *)(tuya_local_key.c_str()), remote_nonce, hmac_payload);
        linked = true;
    }
}

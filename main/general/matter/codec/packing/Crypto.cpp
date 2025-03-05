#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"

//
#include <raii/misc.hpp>
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>

// use part of message header ( messageFlags, sessionId [securityFlags, messageId, u64_node_id] ... )
bytespan_t getNonce(Message const& message) {
    writer_t nonce_w;
    nonce_w.writeByte(reinterpret_cast<uint8_t const&>(message.header.securityFlags));
    nonce_w.writeUInt32(message.header.messageId);
    nonce_w.writeUInt64(message.header.sourceNodeId.value_or(message.header.destNodeId.value_or(0)));
    return nonce_w;
}

//
bytespan_t& MessageCodec::encryptPayload(Message& message,  bytespan_t const& aad, SessionKeys const& keys) {
    decltype(auto) raw = message.rawPayload;
    if (raw->size() && keys.R2Ikeys) {
        // !needs use same bytespace (currently, not implemented)
        auto ext = make_bytes(raw->size() + 16);
        auto tag = bytespan_t(ext->data() + raw->size(), 16);

        //
        mbedtls_ccm_context aes; mbedtls_ccm_init(&aes); auto nonce = getNonce(message);
        checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&keys.R2Ikeys), 128), "Encrypt Key Init Failed");
        checkMbedtlsError(mbedtls_ccm_encrypt_and_tag(&aes,
            raw->size(),
            nonce->data(), nonce->size(),
            aad->data(), aad->size(),
            raw->data(), ext->data(),
            tag->data(), tag->size()
        ), "Encryption Failed");
        mbedtls_ccm_free(&aes);

        //
        return (message.cryptPayload = ext);
    };
    return (message.cryptPayload = raw);
}

//
bytespan_t& MessageCodec::decryptPayload(Message& message,  bytespan_t const& aadWith, SessionKeys const& keys) {
    decltype(auto) ext = message.cryptPayload;
    if (ext->size() && keys.I2Rkeys) {
        auto aad = bytespan_t(aadWith->data(), aadWith->size() - message.cryptPayload->size());
        auto tag = bytespan_t(message.cryptPayload->data() + (message.cryptPayload->size() - 16), 16);
        auto raw = bytespan_t(message.cryptPayload->data(), (message.cryptPayload->size() - 16));

        //
        mbedtls_ccm_context aes; mbedtls_ccm_init(&aes); auto nonce = getNonce(message);
        checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&keys.I2Rkeys), 128), "Decrypt Key Init Failed");
        checkMbedtlsError(mbedtls_ccm_auth_decrypt(&aes,
            raw->size(),
            nonce->data(), nonce->size(),
            aad->data(), aad->size(),
            ext->data(), raw->data(),
            tag->data(), tag->size()
        ), "Decryption Failed");
        mbedtls_ccm_free(&aes);

        //
        return (message.rawPayload = raw);
    };
    return (message.rawPayload = ext);
}

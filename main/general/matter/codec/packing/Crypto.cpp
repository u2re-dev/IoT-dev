#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"

//
#include <raii/misc.hpp>
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>

//
bytespan_t& MessageCodec::encryptPayload(Message& message,  bytespan_t const& aad, SessionKeys const& keys) {
    decltype(auto) rawPayload = message.rawPayload;
    if (rawPayload->size() && keys.R2Ikeys) {
        writer_t nonce_w;
        nonce_w.writeByte(reinterpret_cast<uint8_t const&>(message.header.securityFlags));
        nonce_w.writeUInt32(message.header.messageId);
        nonce_w.writeUInt64(message.header.sourceNodeId.value_or(0));

        //
        auto extended = make_bytes(rawPayload->size() + 16);
        auto tag = bytespan_t(extended->data() + rawPayload->size(), 16);

        //
        mbedtls_ccm_context aes; mbedtls_ccm_init(&aes); bytespan_t nonce = nonce_w;
        checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&keys.R2Ikeys), 128), "Decrypt Key Init Failed");
        checkMbedtlsError(mbedtls_ccm_encrypt_and_tag(&aes,
            extended->size(),
            nonce->data(), nonce->size(),
            aad->data(), aad->size(),
            rawPayload->data(),
            extended->data(),
            tag->data(), tag->size()
        ), "Decryption Failed");
        mbedtls_ccm_free(&aes);

        //
        return (message.cryptPayload = extended);
    };
    return (message.cryptPayload = message.rawPayload);
}

//
bytespan_t& MessageCodec::decryptPayload(Message& message,  bytespan_t const& aadWith, SessionKeys const& keys) {
    if (message.cryptPayload->size() && keys.I2Rkeys) {
        writer_t nonce_w;
        nonce_w.writeByte(reinterpret_cast<uint8_t const&>(message.header.securityFlags));
        nonce_w.writeUInt32(message.header.messageId);
        nonce_w.writeUInt64(message.header.sourceNodeId.value_or(0));

        //
        auto aad = bytespan_t(aadWith->data(), aadWith->size() - message.cryptPayload->size());
        auto tag = bytespan_t(message.cryptPayload->data() + (message.cryptPayload->size() - 16), 16);
        message.rawPayload = bytespan_t(message.cryptPayload->data(), (message.cryptPayload->size() - 16));

        //
        mbedtls_ccm_context aes; mbedtls_ccm_init(&aes); bytespan_t nonce = nonce_w;
        checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&keys.I2Rkeys), 128), "Decrypt Key Init Failed");
        checkMbedtlsError(mbedtls_ccm_auth_decrypt(&aes,
            message.rawPayload->size(),
            nonce->data(), nonce->size(),
            aad->data(), aad->size(),
            message.rawPayload->data(),
            message.rawPayload->data(),
            tag->data(), tag->size()
        ), "Decryption Failed");
        mbedtls_ccm_free(&aes);
        return message.rawPayload;
    };
    return (message.rawPayload = message.cryptPayload);
}

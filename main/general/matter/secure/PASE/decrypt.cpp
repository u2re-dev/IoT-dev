#ifndef D06B583A_09D8_4619_B336_2BFFFD2BB5E2
#define D06B583A_09D8_4619_B336_2BFFFD2BB5E2

//
#include "../PASE.hpp"

//
SessionKeys& PASE::makeSessionKeys() {
    auto info = hex::s2b("SessionKeys");
    auto keys = crypto::hkdf_len(hkdf.Ke, info);

    // TODO: better interpret code
    return (sessionKeys = SessionKeys {
        *reinterpret_cast<intx::uint128*>(keys->data()),
        *reinterpret_cast<intx::uint128*>(keys->data() + 16),
        *reinterpret_cast<intx::uint128*>(keys->data() + 32)
    });
}

//
bytespan_t& PASE::decryptPayload(Message& message,  bytespan_t const& bytes) const {
    if (message.rawPayload->size() && sessionKeys.I2Rkeys) {
        writer_t nonce_w;
        nonce_w.writeByte(reinterpret_cast<uint8_t const&>(message.header.securityFlags));
        nonce_w.writeUInt32(message.header.messageId);
        nonce_w.writeUInt64(message.header.sourceNodeId.value_or(0));

        //
        auto aad = bytespan_t(bytes->data(), bytes->size() - message.rawPayload->size());
        auto tag = bytespan_t(message.rawPayload->data() + (message.rawPayload->size() - 16), 16);
        message.rawPayload = bytespan_t(message.rawPayload->data(), (message.rawPayload->size() - 16));

        //
        mbedtls_ccm_context aes; mbedtls_ccm_init(&aes); bytespan_t nonce = nonce_w;
        checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&sessionKeys.I2Rkeys), 128), "Decrypt Key Init Failed");
        checkMbedtlsError(mbedtls_ccm_auth_decrypt(&aes, //MBEDTLS_CCM_DECRYPT,
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
    return message.rawPayload;
}

//
#endif

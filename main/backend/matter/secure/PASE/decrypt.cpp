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
Message& PASE::decryptPayload(Message& message,  bytespan_t const& bytes) const {
    if (message.header.sessionId != 0 && message.rawPayload->size() && sessionKeys.I2Rkeys) {
        auto aad = std::span<uint8_t>(bytes->begin(), bytes->end() - message.rawPayload->size());

        //
        writer_t nonce_w;
        nonce_w.writeByte(message.header.securityFlags);
        nonce_w.writeUInt32(message.header.messageId);
        nonce_w.writeUInt64(message.header.sourceNodeId.value_or(0));
        bytespan_t nonce = nonce_w;

        //
        mbedtls_ccm_context aes;
        mbedtls_ccm_init(&aes);

        // set keys
        checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<uint8_t const*>(&sessionKeys.I2Rkeys), 128), "Decrypt Key Init Failed");

        //
        //auto output = make_bytes(message.rawPayload->size() - 16);
        checkMbedtlsError(mbedtls_ccm_auth_decrypt(&aes, //MBEDTLS_CCM_DECRYPT,
            (message.rawPayload->size() - 16),
            nonce->data(),
            nonce->size(),
            aad.data(),
            aad.size(),
            //message.rawPayload->size() - 16,//aad.size(),
            message.rawPayload->data(),
            message.rawPayload->data(),
            //output->data(),
            message.rawPayload->data() + (message.rawPayload->size() - 16),
            16
        ), "Decryption Failed");
        mbedtls_ccm_free(&aes);

        //
        std::cout << "Decoded Payload:" << std::endl;
        std::cout << hex::b2h(message.rawPayload) << std::endl;
    };
    return message;
}

#endif /* D06B583A_09D8_4619_B336_2BFFFD2BB5E2 */

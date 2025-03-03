#pragma once

//
#include <std/types.hpp>
#include "./utils.hpp"

//
namespace tc {
    static const char* local_nonce = "0123456789abcdef";
    struct TuyaCmd { uint32_t SEQ_NO = 0, CMD_ID = 0; bigint_t HMAC = 0; };

    // tuya 3.4
    inline uint32_t getTuyaCmd(uint8_t* encrypted_code) { return bswap32(*reinterpret_cast<uint32_t const*>(encrypted_code+8)); }
    inline uint32_t getTuyaSeq(uint8_t* encrypted_code) { return bswap32(*reinterpret_cast<uint32_t const*>(encrypted_code+4)); }
    inline bytespan_t getTuyaPayload(bytespan_t const& encrypted_code) { return bytespan_t(encrypted_code->data()+20, bswap32(*reinterpret_cast<uint32_t const*>(encrypted_code+12))); }

    //
    inline uint32_t computePayloadSize(uint32_t payloadLen, bool hmac) { return payloadLen + (hmac ? 32 : 4) + 4; }
    inline uint32_t computeCodeSize(uint32_t payloadLen, bool hmac) { return 16 + computePayloadSize(payloadLen, hmac); }

    // for protocol 3.4, remote_nonce is encrypted, HMAC i.e. hmac_key
    bigint_t   encryptDataECB(bigint_t const& key,  bigint_t   const& data);
    bigint_t   decryptDataECB(bigint_t const& key,  bigint_t   const& data);

    //
    bytespan_t encryptDataECB(bigint_t const& key,  bytespan_t const& data, const bool usePadding = true);
    bytespan_t decryptDataECB(bigint_t const& key,  bytespan_t const& data);

    //
    bytespan_t encode_remote_hmac(bigint_t const& original_key, bigint_t const& remote_nonce);
    bigint_t   encode_hmac_key   (bigint_t const& original_key, bigint_t const& remote_nonce);

    // encode message code
    bytespan_t prepareTuyaCode(size_t const& length, TuyaCmd const& cmdDesc,  bytes_t& output);
    bytespan_t checksumTuyaCode(bytespan_t& code, bigint_t const& HMAC = 0);
    bytespan_t encodeTuyaCode(bytespan_t const& encrypted_data, TuyaCmd const& cmdDesc, bytes_t& output);

    //
    // ESP32-S3 or CardPuter won't support native AES GCM, but tuya protocol 3.5 requires it
    //bytespan_t decryptDataGCM(bigint_t const& key,  bytespan_t const& data);
    //bytespan_t encryptDataGCM(bigint_t const& key,  bytespan_t const& data);

    //
    //size_t prepareTuyaCode35(size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr);
    //size_t encodeTuyaCode35(uint8_t* encrypted_data, size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr);
    //std::array<uint32_t, 2> prepareJSON(uint8_t*data, size_t& length, char const* protocolVersion, uint8_t* output);
};

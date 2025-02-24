#pragma once

//
#include "../../std/std.hpp"
#include "../../std/utils.hpp"

//
namespace tc {

    //
    static const char* local_nonce = "0123456789abcdef";

    //
    struct TuyaCmd {
        uint32_t SEQ_NO; //= 0;
        uint32_t CMD_ID;// = 0;
        uint8_t* HMAC;// = nullptr;
    };

    // tuya 3.4
    inline uint32_t getTuyaCmd(uint8_t* encrypted_code) { return bswap32(*(uint32_t const*)(encrypted_code+8)); }
    inline uint32_t getTuyaSeq(uint8_t* encrypted_code) { return bswap32(*(uint32_t const*)(encrypted_code+4)); }
    inline uint8_t* getTuyaPayload(uint8_t* encrypted_code, size_t& length) { length = bswap32(*(uint32_t const*)(encrypted_code+12)); return (encrypted_code+20); }
    inline uint32_t computePayloadSize(uint32_t payloadLen, bool hmac) { return payloadLen + (hmac ? 32 : 4) + 4; }
    inline uint32_t computeCodeSize(uint32_t payloadLen, bool hmac) { return 16 + computePayloadSize(payloadLen, hmac); }

    //
    uint8_t* encryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output = nullptr, const bool usePadding = true);
    uint8_t* decryptDataECB(uint8_t* key,  uint8_t* data, size_t& length,  uint8_t* output = nullptr);

    // for protocol 3.4, remote_nonce is encrypted, HMAC i.e. hmac_key
    uint8_t* encode_remote_hmac(uint8_t* original_key, uint8_t* remote_nonce,  uint8_t* remote_hmac = nullptr);
    uint8_t* encode_hmac_key(uint8_t* original_key, uint8_t* remote_nonce,  uint8_t* hmac_key = nullptr);

    // encode message code
    size_t prepareTuyaCode(size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr);
    size_t checksumTuyaCode(uint8_t* code, uint8_t* HMAC);
    size_t encodeTuyaCode(uint8_t* encrypted_data, size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr);

    //
#ifdef TUYA_35_SUPPORT
    // ESP32-S3 or CardPuter won't support native AES GCM, but tuya protocol 3.5 requires it
    uint8_t* decryptDataGCM(uint8_t* key, uint8_t* data, size_t& length,  uint8_t* output = nullptr);
    uint8_t* encryptDataGCM(uint8_t* key, uint8_t* data, size_t& length,  uint8_t* output = nullptr);
#endif

    //
#ifdef TUYA_35_SUPPORT
    size_t prepareTuyaCode35(size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr);
    size_t encodeTuyaCode35(uint8_t* encrypted_data, size_t& length, TuyaCmd const& cmdDesc = {}, uint8_t* output = nullptr);
#endif

    //
#ifdef ENABLE_UNUSED
    std::array<uint32_t, 2> prepareJSON(uint8_t*data, size_t& length, char const* protocolVersion, uint8_t* output);
#endif

};

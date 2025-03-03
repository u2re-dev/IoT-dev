#include "../session.hpp"

//
namespace th {
    //
    uint8_t* TuyaSession::encodeMessage(uint cmd, uint8_t *data, size_t &keyLen) {
        outLen = tc::encodeTuyaCode(data, keyLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);
        if (outLen > 0)
        { // debug-log
            DebugLog("Sent Code");
            DebugCode(outBuffer, outLen);
        }

        // TODO: use bytespan_t
        return outLen > 0 ? outBuffer : nullptr;
    }

    //
#ifdef USE_ARDUINO_JSON
    uint8_t*  TuyaSession::encodeJSON(uint const& cmd, ArduinoJson::JsonDocument &doc)
#else
    uint8_t*  TuyaSession::encodeJSON(uint const& cmd, json &doc)
#endif
    {   // protocol 3.4 specific
#ifdef USE_ARDUINO_JSON
        size_t jsonLen = ArduinoJson::measureJson(doc);
#else
        decltype(auto) bset = doc.dump();
        size_t jsonLen = bset.size();
#endif

    //
#ifdef TUYA_35_SUPPORT // protocol 3.5 specific
        const size_t HEADER_OFFSET = 18;
        size_t withHeadLen = jsonLen + 15;
        size_t encryptLen = (((withHeadLen + 16) >> 4) << 4);
        size_t prepareLen = encryptLen + 12 + 16;
#else // protocol 3.4 specific
        const size_t HEADER_OFFSET = 16;
        size_t withHeadLen = jsonLen + 15;
        size_t encryptLen = ((withHeadLen + 16) >> 4) << 4; // 'encryptLen = (((jsonLen + 16) >> 4) << 4)' if tuya protocol is 3.3
        size_t prepareLen = encryptLen;                     // 'prepareLen = encryptLen + 15' if tuya protocol is 3.3
#endif

        //
        outLen = tc::prepareTuyaCode(prepareLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);

        //
#ifdef TUYA_35_SUPPORT
        uint8_t *enc = outBuffer + HEADER_OFFSET;
        uint8_t *payload = enc + 12;
#else
        uint8_t *payload = outBuffer + HEADER_OFFSET;
        uint8_t *enc = payload; // 'payload + 15' if tuya protocol 3.3
#endif

        //
#ifdef USE_ARDUINO_JSON
        serializeJson(doc, payload + 15, std::min(size_t(512), jsonLen));
#else
        memcpy(payload + 15, bset.c_str(), std::min(size_t(512), jsonLen));
#endif

        //
        for (uint i = 0; i < 15; i++) { payload[i] = 0; };
#ifdef TUYA_35_SUPPORT                                                   // protocol 3.5 specific
        memcpy(payload, "3.5", 3);                                       // protocol version
        memcpy(enc, std::to_string(getUnixTime() * 100ull).c_str(), 12); // generate IV
        tc::encryptDataGCM(hmac_key, enc, encryptLen);
#else
        memcpy(payload, "3.4", 3);
        tc::encryptDataECB(hmac_key, enc, encryptLen);
        tc::checksumTuyaCode(outBuffer, hmac_key);
#endif

        // TODO: use bytespan_t
        return outLen > 0 ? outBuffer : nullptr;
    }


    // when `code == 0x8`
    uint8_t* TuyaSession::handleJson(uint8_t *payload) {
        const size_t data_offset = 15;
        size_t json_len = payloadLength;

        // protocol 3.4 specific
        tc::decryptDataECB(hmac_key, payload, json_len, payload);
        uint8_t *json_part = payload + data_offset;
        json_len -= data_offset;

        // protocol 3.3 specific
        // tc::decryptDataECB(hmac_key, payload + data_offset, json_len - data_offset, payload + data_offset);
        // uint8_t* json_part = payload + data_offset; json_len -= data_offset;

        //
    #ifdef USE_ARDUINO_JSON
        deserializeJson(current, json_part, json_len);
    #else
        current = json::parse(json_part, json_part + json_len);
    #endif
    }
}

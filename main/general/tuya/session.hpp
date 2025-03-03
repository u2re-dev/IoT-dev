#pragma once

//
#include <std/types.hpp>
#include <cstdint>
#include "./codec/libtuya.hpp"

//
#ifdef USE_ARDUINO_JSON
#include <ArduinoJson.hpp>
#else
#include <json/json.hpp>
using json = nlohmann::json;
#endif



//
namespace th
{
    //
    class TuyaSession {
    protected:
        uint32_t SEQ_NO = 1;

        //
#ifdef USE_ARDUINO_JSON
        ArduinoJson::JsonDocument blank;
        ArduinoJson::JsonDocument data;
        ArduinoJson::JsonDocument sending;
        ArduinoJson::JsonDocument current;
#else
        json blank = {};
        json data = {};
        json sending = {};
        json current = {};
#endif
        bool linked = false;
        std::string tuya_local_key = "";
        std::string tuya_local_ip = "";
        std::string device_id = "";
        std::string device_uid = "";

        //
        bigint_t local_nonce_crypt = {};
        bigint_t hmac_key = {};

        //
        bytespan_t hmac = {};
        bytespan_t hmac_payload = {};

        //
        bytes_t inBuffer = {};
        bytes_t outBuffer = {};

        //
    public:
        TuyaSession() { // TODO: replace to bigint
            hmac_payload = make_bytes(16 + 16 + 12);
            inBuffer  = make_bytes(512);
            outBuffer = make_bytes(512);
            SEQ_NO = 1; linked = false;
        }

        //
        void init(std::string tuya_local_key, std::string device_id, std::string device_uid);

        //
        bigint_t encodeLocalNonce();
        bytespan_t encodeMessage(uint const& cmd, bytespan_t const& data);
        bytespan_t handleSignal(bytespan_t const& inBuffer);

        //
        bytespan_t handleJson(bytespan_t const& payload);
        bytespan_t sharedNonce(bigint_t const& remote_nonce);
        bigint_t resolveKey(bigint_t const& remote_nonce);

        //
#ifdef USE_ARDUINO_JSON
        bytes_t getDPS() { return encodeJSON(0x10, blank); }
        bytes_t setDPS(ArduinoJson::JsonObject const &dps);
        bytes_t encodeJSON(uint const& cmd, ArduinoJson::JsonDocument &doc);
#else
        bytes_t getDPS() { return encodeJSON(0x10, blank); }
        bytes_t setDPS(json const &dps);
        bytes_t encodeJSON(uint const& cmd, json &doc);
#endif

        //
        bool available() { return linked; }
    };

};

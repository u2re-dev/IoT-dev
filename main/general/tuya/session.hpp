#pragma once

//
#include <std/types.hpp>
#include <cstdint>

//
#include "./codec/libtuya.hpp"

//
#ifdef USE_ARDUINO_JSON
#include <ArduinoJson.hpp>
#else
#include <json/json.hpp>
using json = nlohmann::json;
#endif

//
namespace th { //
    class TuyaSession {
    protected:
        uint32_t SEQ_NO = 1;
        bool linked = false;

        //
#ifdef USE_ARDUINO_JSON
        ArduinoJson::JsonDocument blank, data, sending, current;
#else
        json blank = {}, data = {}, sending = {}, current = {};
#endif

        //
        std::string tuya_local_key = "";
        std::string device_id = "";
        std::string device_uid = "";

        //
        tc::block_t local_nonce_crypt = 0;
        tc::block_t hmac_key = 0;

        //
        bytespan_t hmac   = {};
        bytes_t outBuffer = {};

        //
    public:
        TuyaSession() {
            SEQ_NO  = 1; linked = false;
            outBuffer = make_bytes(512);
        }

        //
        TuyaSession(std::string device_id, std::string tuya_local_key, std::string device_uid = "") {
            SEQ_NO  = 1; linked = false;
            outBuffer = make_bytes(512);
            init(device_id, tuya_local_key, device_uid);
        }

        //
        void handleJson(bytespan_t const& payload);
        void init(std::string device_id, std::string tuya_local_key, std::string device_uid = "");
        bool available() { return linked; }

        //
        bytespan_t encodeLocalNonce();
        bytespan_t encodeMessage(uint const& cmd, bytespan_t const& data);
        bytespan_t handleSignal(bytespan_t const& inBuffer);
        bytespan_t sharedNonce(tc::block_t const& remote_nonce);
        tc::block_t resolveKey(tc::block_t const& remote_nonce);

        //
#ifdef USE_ARDUINO_JSON
        bytespan_t getDPS() { return encodeJSON(0x10, blank); }
        bytespan_t setDPS(ArduinoJson::JsonObject const &dps);
        bytespan_t encodeJSON(uint const& cmd, ArduinoJson::JsonDocument &doc);
#else
        bytespan_t getDPS() { return encodeJSON(0x10, blank); }
        bytespan_t setDPS(json const &dps);
        bytespan_t encodeJSON(uint const& cmd, json &doc);
#endif
    };

};

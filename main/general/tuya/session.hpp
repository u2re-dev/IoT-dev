#pragma once

//
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

        std::string tuya_local_key = "";
        std::string tuya_local_ip = "";
        std::string device_id = "";
        std::string device_uid = "";

        // TODO: replace to bigint
        uint8_t *tmp = nullptr;      // for sending local_nonce
        uint8_t *hmac = nullptr;     // length = 32 or 48
        uint8_t *hmac_key = nullptr; // length = 16
        uint8_t *hmac_payload = nullptr;

        // data buffer
        uint8_t *inBuffer = nullptr;
        uint8_t *outBuffer = nullptr;
        size_t inLen = 0, outLen = 0;
        bool linked = false;

        //
    public:
        TuyaSession() { // TODO: replace to bigint
            tmp = (uint8_t *)calloc(1, 16);
            hmac_key = (uint8_t *)calloc(1, 16);
            hmac_payload = (uint8_t *)calloc(1, 16 + 16 + 12);

            // TODO: auto extension when required
            inBuffer  = (uint8_t *)calloc(1, 512);
            outBuffer = (uint8_t *)calloc(1, 512);
            SEQ_NO = 1;
            linked = false;
        }

        //
        void init(std::string tuya_local_key, std::string device_id, std::string device_uid);
        void encodeMessage(uint cmd, uint8_t *data, size_t &keyLen);
        void encodeLocalNonce();
        void handleSignal();

        //
        void handleJson(uint8_t const* payload);
        void sharedNonce(uint8_t const* remote_nonce);
        void resolveKey(uint8_t const* remote_nonce);

        //
#ifdef USE_ARDUINO_JSON
        void getDPS() { sendJSON(0x10, blank); }
        void setDPS(ArduinoJson::JsonObject const &dps);
        void sendJSON(uint cmd, ArduinoJson::JsonDocument &doc);
#else
        void getDPS() { sendJSON(0x10, blank); }
        void setDPS(json const &dps);
        void sendJSON(uint cmd, json &doc);
#endif

        //
        bool available() { return linked; }
    };

};

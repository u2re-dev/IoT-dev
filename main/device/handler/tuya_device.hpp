#pragma once

//
#include <cstdint>
#include <ArduinoJson.hpp>

//
#include "../../std/libtuya.hpp"
#include "../hal/network.hpp"
#include "../m5stack/rtc.hpp"

//
namespace th {

    //
    class TuyaDevice34 {
        protected:
        uint32_t SEQ_NO = 1;
        WiFiClient client;

        //
        ArduinoJson::JsonDocument blank;
        ArduinoJson::JsonDocument data;
        ArduinoJson::JsonDocument sending;
        ArduinoJson::JsonDocument current;
        std::string tuya_local_key = "";
        std::string tuya_local_ip = "";
        std::string device_id = "";
        std::string device_uid = "";

        //
        uint8_t* tmp = nullptr; // for sending local_nonce
        uint8_t* hmac = nullptr; // length = 32 or 48
        uint8_t* hmac_key = nullptr; // length = 16
        uint8_t* hmac_payload = nullptr;

        // data buffer
        uint8_t* inBuffer = nullptr;
        uint8_t* outBuffer = nullptr;
        size_t inLen = 0, outLen = 0;
        bool linked = false;

        //
        public:
        TuyaDevice34() {
            tmp = (uint8_t*)calloc(1, 16);
            hmac_key = (uint8_t*)calloc(1, 16);
            hmac_payload = (uint8_t*)calloc(1, 16 + 16 + 12);

            // TODO: auto extension when required
            inBuffer = (uint8_t*)calloc(1, 512);
            outBuffer = (uint8_t*)calloc(1, 512);
            SEQ_NO = 1; linked = false;
        }

        //
        void connectDevice(std::string tuya_local_ip, std::string tuya_local_key, std::string device_id, std::string device_uid);
        void sendMessage(uint cmd, uint8_t* data, size_t& keyLen);
        void sendLocalNonce();
        void getDPS() { sendJSON(0x10, blank); }
        void setDPS(ArduinoJson::JsonObject const& dps);
        void sendJSON(uint cmd, ArduinoJson::JsonDocument& doc);

        //
        bool available() { return linked; }
        bool connected() {  return client.connected(); }
        void handleSignal();
    };

};

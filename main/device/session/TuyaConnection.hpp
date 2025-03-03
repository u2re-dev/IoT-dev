#pragma once

//
#include <cstdint>

//
#include <tuya/libtuya.hpp>
#include <device/hal/network.hpp>
#include <device/hal/rtc.hpp>

//
#ifdef USE_ARDUINO_JSON
#include <ArduinoJson.hpp>
#else
#include <json/json.hpp>
using json = nlohmann::json;
#endif

//
namespace th {
    //
    class TuyaConnection { protected:
#ifdef USE_ARDUINO
        WiFiClient client;
#endif

        // data buffer
        uint8_t *inBuffer = nullptr;
        uint8_t *outBuffer = nullptr;
        size_t inLen = 0, outLen = 0;

        //
    public:
        TuyaConnection() {}

        //
        void connectDevice(std::string tuya_local_ip, std::string tuya_local_key, std::string device_id, std::string device_uid);
        void sendMessage(uint cmd, uint8_t *data, size_t &keyLen);
        void sendLocalNonce();
        void handleSignal();
        bool available() { return linked; }
        bool connected() {
#ifdef USE_ARDUINO
            return client.connected();
#else
            return false;
#endif
        }
    };

};

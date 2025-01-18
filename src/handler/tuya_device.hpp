#pragma once

//
#include <cstdint>
#include <hal/network.hpp>
#include <std/libtuya.hpp>
#include <ArduinoJson.hpp>
#include <m5stack/rtc.hpp>

//
namespace th {

    //
    class TuyaDevice34 {
        protected:
        uint32_t SEQ_NO = 1;
        WiFiClient client;

        //
        ArduinoJson::JsonDocument current;
        std::string tuya_local_key = "";
        std::string tuya_local_ip = "";
        std::string device_id = "";
        std::string device_uid = "";

        //
        uint8_t* tmp = nullptr; // for sending local_nonce
        uint8_t* hmac = nullptr; // length = 32 or 48
        uint8_t* hmac_key = nullptr; // length = 16

        // data buffer
        uint8_t* inBuffer = nullptr;
        uint8_t* outBuffer = nullptr;
        size_t inLen = 0, outLen = 0;

        //
        public:
        TuyaDevice34() {
            tmp = (uint8_t*)calloc(1, 16);
            hmac_key = (uint8_t*)calloc(1, 16);

            // TODO: auto extension when required
            inBuffer = (uint8_t*)calloc(1, 512);
            outBuffer = (uint8_t*)calloc(1, 512);
            SEQ_NO = 1;
        }

        //
        void connectDevice(std::string tuya_local_ip, std::string tuya_local_key, std::string device_id, std::string device_uid) {
            this->tuya_local_ip  = tuya_local_ip;
            this->tuya_local_key = tuya_local_key;
            this->device_id  = device_id;
            this->device_uid = device_uid;

            // parse from string and use it
            uint8_t ip[4] = {0,0,0,0}; ipv4_parse((uint8_t*)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);
            connectToDevice(client, IPAddress(ip));
            memcpy(hmac_key, tuya_local_key.c_str(), 16);
            SEQ_NO = 1;
        }

        //
        void sendMessage(uint cmd, uint8_t* data, size_t& keyLen) {
            outLen = tc::encodeTuyaCode(data, keyLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);
            if (outLen > 0) { // debug-log
                DebugLog("Sent Code");
                DebugCode(outBuffer, outLen);
            }
            if (outLen > 0) {
                waitAndSend(client, outBuffer, outLen);
            }
        }

        //
        void sendLocalNonce() {
            if (client.connected()) {
                size_t keyLen = 16; // say hello with local_nonce with local_key encryption and checksum
                sendMessage(0x3u, tc::encryptDataECB((uint8_t*)tuya_local_key.c_str(), (uint8_t*)tc::local_nonce, keyLen, tmp, false), keyLen);
            }
        }

        // protocol 3.4 specific
        void sendJSON(uint cmd, ArduinoJson::JsonDocument& doc) {
            // write into json actual info
            doc["t"] = getUnixTime();
            doc["devId"] = device_id;
            doc["uid"] = device_uid;
            doc["gwId"] = device_id;

            // 
            const size_t HEADER_OFFSET = 16;

            //
            size_t jsonLen = ArduinoJson::measureJson(doc);

            // protocol 3.4 specific
            size_t withHeadLen = jsonLen + 15, encryptLen = ((withHeadLen + 16) >> 4) << 4;
            outLen = tc::prepareTuyaCode(encryptLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);

            //
            uint8_t* payload = outBuffer + HEADER_OFFSET;
            serializeJson(doc, payload + 15, std::min(size_t(512), jsonLen));
            memcpy(payload, "3.4", 3);

            // protocol 3.4 specific
            tc::encryptDataECB(hmac_key, payload, withHeadLen, payload);

            //
            tc::checksumTuyaCode(outBuffer, hmac_key);

            //
            if (outLen > 0) { // debug-log
                DebugLog("Sent Code");
                DebugCode(outBuffer, outLen);
            }
            if (outLen > 0) {
                waitAndSend(client, outBuffer, outLen);
            }

            // protocol 3.3 specific
            //size_t withHeadLen = jsonLen + 15, encryptLen = (((jsonLen + 16) >> 4) << 4) + 15;
            //tc::encryptDataECB(hmac_key, payload + 15, jsonLen, payload);
        }

        //
        bool connected() {
            return client.connected();
        }

        //
        void handleSignal() {
            if (!client.connected()) {
                return;
            }

            //
            waitForReceive(client, inBuffer, inLen, 100);
            if (inLen > 0) {
                DebugLog("Received Code");
                DebugCode(inBuffer, inLen);

                //
                size_t payloadLength = 0;
                uint8_t* payload = tc::getTuyaPayload(inBuffer, payloadLength);

                //
                const auto code = tc::getTuyaCmd(inBuffer);
                if (code == 0x4) {
                    // make linked with local remote hmac
                    size_t hmac_len = 48; hmac = (uint8_t*)calloc(1, hmac_len);
                    tc::encode_remote_hmac((uint8_t*)(tuya_local_key.c_str()), payload, hmac);

                    // send computed hmac
                    sendMessage(0x5u, hmac, hmac_len);

                    //
                    tc::encode_hmac_key((uint8_t*)(tuya_local_key.c_str()), payload, hmac_key);
                }
                if (code == 0x8) {
                    const size_t data_offset = 15;
                    size_t json_len = payloadLength;

                    // protocol 3.4 specific
                    tc::decryptDataECB(hmac_key, payload, json_len, payload);
                    uint8_t* json_part = payload + data_offset; json_len -= data_offset;

                    // protocol 3.3 specific
                    //tc::decryptDataECB(hmac_key, payload + data_offset, json_len - data_offset, payload + data_offset);
                    //uint8_t* json_part = payload + data_offset; json_len -= data_offset;

                    //
                    deserializeJson(current, json_part, json_len);
                }
            }
            inLen = 0;
        }
    };

};

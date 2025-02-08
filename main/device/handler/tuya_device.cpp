#include "./tuya_device.hpp"

//
namespace th {

    //
    void TuyaDevice34::connectDevice(std::string tuya_local_ip, std::string tuya_local_key, std::string device_id, std::string device_uid) {
        this->tuya_local_ip  = tuya_local_ip;
        this->tuya_local_key = tuya_local_key;
        this->device_id  = device_id;
        this->device_uid = device_uid;

        // padding from IV
        hmac_key = hmac_payload + 12;

        // parse from string and use it
        uint8_t ip[4] = {0,0,0,0}; ipv4_parse((uint8_t*)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);

#ifdef USE_ARDUINO
        connectToDevice(client, IPAddress(ip));
#endif

        memcpy(hmac_key, tuya_local_key.c_str(), 16);
        SEQ_NO = 1; linked = false;
    }

    //
    void TuyaDevice34::sendMessage(uint cmd, uint8_t* data, size_t& keyLen) {
        outLen = tc::encodeTuyaCode(data, keyLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);
        if (outLen > 0) { // debug-log
            DebugLog("Sent Code");
            DebugCode(outBuffer, outLen);
        }
#ifdef USE_ARDUINO
        if (outLen > 0) {
            waitAndSend(client, outBuffer, outLen);
        }
#endif
    }

    //
    void TuyaDevice34::sendLocalNonce() {
#ifdef USE_ARDUINO
        if (client.connected()) {
            size_t keyLen = 16; // say hello with local_nonce with local_key encryption and checksum
            sendMessage(0x3u, tc::encryptDataECB((uint8_t*)tuya_local_key.c_str(), (uint8_t*)tc::local_nonce, keyLen, tmp, false), keyLen);
        }
#endif
    }

    //
#ifdef USE_ARDUINO_JSON
    void TuyaDevice34::setDPS(ArduinoJson::JsonObject const& dps) {
        sending["protocol"] = 5;
        sending["t"] = uint64_t(getUnixTime()) * 1000ull;

        //
        data["dps"] = dps;
        sending["data"] = data.as<ArduinoJson::JsonObject>();

        // protocol 3.3
        //sending["devId"] = device_id;
        //sending["uid"] = device_uid;
        //sending["dps"] = dps;
        //sendJSON(0x7, sending);

        // protocol 3.4
        sendJSON(0xd, sending);
    }

    // protocol 3.4 specific
    void TuyaDevice34::sendJSON(uint cmd, ArduinoJson::JsonDocument& doc) {
        //
        size_t jsonLen = ArduinoJson::measureJson(doc);

#ifdef TUYA_35_SUPPORT
        // protocol 3.5 specific
        const size_t HEADER_OFFSET = 18;
        size_t withHeadLen = jsonLen + 15;
        size_t encryptLen = (((withHeadLen + 16) >> 4) << 4);
        size_t prepareLen = encryptLen + 12 + 16;
#else   // protocol 3.4 specific
        const size_t HEADER_OFFSET = 16;
        size_t withHeadLen = jsonLen + 15;
        size_t encryptLen  = ((withHeadLen + 16) >> 4) << 4; // 'encryptLen = (((jsonLen + 16) >> 4) << 4)' if tuya protocol is 3.3
        size_t prepareLen  = encryptLen;                     // 'prepareLen = encryptLen + 15' if tuya protocol is 3.3
#endif

        //
        outLen = tc::prepareTuyaCode(prepareLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);

        //
#ifdef TUYA_35_SUPPORT 
        uint8_t* enc = outBuffer + HEADER_OFFSET;
        uint8_t* payload = enc + 12;
#else
        uint8_t* payload = outBuffer + HEADER_OFFSET;
        uint8_t* enc = payload; // 'payload + 15' if tuya protocol 3.3
#endif

        //
        serializeJson(doc, payload + 15, std::min(size_t(512), jsonLen));
        for (uint i=0;i<15;i++) { payload[i] = 0; };

        //
#ifdef TUYA_35_SUPPORT // protocol 3.5 specific
        memcpy(payload, "3.5", 3); // protocol version
        memcpy(enc, std::to_string(getUnixTime() * 100ull).c_str(), 12); // generate IV
        tc::encryptDataGCM(hmac_key, enc, encryptLen);
#else
        memcpy(payload, "3.4", 3);
        tc::encryptDataECB(hmac_key, enc, encryptLen);
        tc::checksumTuyaCode(outBuffer, hmac_key);
#endif

        // ===== //
        // debug //
        // ===== //

        if (outLen > 0) { // debug-log
            DebugLog("Sent Code");
            DebugCode(outBuffer, outLen);
        }

        if (outLen > 0) {
            waitAndSend(client, outBuffer, outLen);
        }
    }
#endif

    //
    void TuyaDevice34::handleSignal() {
#ifdef USE_ARDUINO
        if (!client.connected()) { return; };
        waitForReceive(client, inBuffer, inLen, 100);
#endif

        //
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
                hmac_key = hmac_payload + 12;

                //
                tc::encode_remote_hmac(hmac_key, payload, hmac);

                // send computed hmac
                sendMessage(0x5u, hmac, hmac_len);

                // protocol 3.4 specific
                tc::encode_hmac_key((uint8_t*)(tuya_local_key.c_str()), payload, hmac_payload);

                // protocol 3.4 link status
                linked = true;
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
#ifdef USE_ARDUINO_JSON
                deserializeJson(current, json_part, json_len);
#endif
            }
        }
        inLen = 0;
    }

};

#include "./tuya_device.hpp"

//
namespace th {

    //
    void TuyaDevice34::connectDevice(std::string tuya_local_ip, std::string tuya_local_key, std::string device_id, std::string device_uid) {
        this->tuya_local_ip  = tuya_local_ip;
        this->tuya_local_key = tuya_local_key;
        this->device_id  = device_id;
        this->device_uid = device_uid;

        // parse from string and use it
        uint8_t ip[4] = {0,0,0,0}; ipv4_parse((uint8_t*)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);
        connectToDevice(client, IPAddress(ip));
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
        if (outLen > 0) {
            waitAndSend(client, outBuffer, outLen);
        }
    }

    //
    void TuyaDevice34::sendLocalNonce() {
        if (client.connected()) {
            size_t keyLen = 16; // say hello with local_nonce with local_key encryption and checksum
            sendMessage(0x3u, tc::encryptDataECB((uint8_t*)tuya_local_key.c_str(), (uint8_t*)tc::local_nonce, keyLen, tmp, false), keyLen);
        }
    }

    //
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
        // write into json actual info

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
        for (uint i=0;i<15;i++) { payload[i] = 0; };
        memcpy(payload, "3.4", 3);

        //
        DebugLog("Sent Code");
        DebugCode(outBuffer, outLen);
        DebugLog((char*)(payload+15));

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

    void TuyaDevice34::handleSignal() {
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
                tc::encode_remote_hmac(hmac_key, payload, hmac);

                // send computed hmac
                sendMessage(0x5u, hmac, hmac_len);

                //
                tc::encode_hmac_key((uint8_t*)(tuya_local_key.c_str()), payload, hmac_key);

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
                deserializeJson(current, json_part, json_len);
            }
        }
        inLen = 0;
    }

};

#include "./TuyaConnection.hpp"

//
namespace th {
    //
    void TuyaConnection::connectDevice(std::string tuya_local_ip/*, std::string tuya_local_key, std::string device_id, std::string device_uid*/) {
        // parse from string and use it
        uint8_t ip[4] = {0, 0, 0, 0};
        ipv4_parse((uint8_t *)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);

#ifdef USE_ARDUINO
        connectToDevice(client, IPAddress(ip));
#endif
    }

    //
    void TuyaConnection::sendMessage(uint cmd, uint8_t *data, size_t &keyLen)
    {
        outLen = tc::encodeTuyaCode(data, keyLen, tc::TuyaCmd{SEQ_NO++, cmd, hmac_key}, outBuffer);
        if (outLen > 0)
        { // debug-log
            DebugLog("Sent Code");
            DebugCode(outBuffer, outLen);
        }
#ifdef USE_ARDUINO
        if (outLen > 0)
        {
            waitAndSend(client, outBuffer, outLen);
        }
#endif
    }

#ifdef USE_ARDUINO_JSON
    void TuyaConnection::sendJSON(uint const& cmd, ArduinoJson::JsonDocument &doc)
#else
    void TuyaConnection::sendJSON(uint const& cmd, json &doc)
#endif
    {
        //if (outLen > 0) { DebugLog("Sent Code"); DebugCode(outBuffer, outLen); }
        if (outLen > 0) { waitAndSend(client, outBuffer, outLen); }
    }

    //
    void TuyaConnection::handleSignal()
    {
#ifdef USE_ARDUINO
        if (!client.connected()) return;
        waitForReceive(client, inBuffer, inLen, 100);
#endif

        inLen = 0;
    }
};

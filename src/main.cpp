#define VERY_LARGE_STRING_LENGTH 8000

//
#define NTP_TIMEZONE  "UTC+7"

//
#include <std/std.hpp>
#include "hal/interface.hpp"
#include "persistent/nv_typed.hpp"

//
#include <esp32-hal-gpio.h>
#include <HardwareSerial.h>
#include <M5Unified.hpp>
#include "hal/network.hpp"
#include <WiFi.h>
#include "m5stack/rtc.hpp"
#include "std/libtuya.hpp"
#include "private.hpp"
#include <ArduinoJson.hpp>

//
void IOTask() {

};

//
void loop(void) {};
void setup(void)
{
    setCpuFrequencyMhz(240);

    //
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);

    //
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nv::storage.begin("nvs", false);
    std::thread IOTaskThread = std::thread(IOTask);

    //
    initWiFi();

    //
    WiFiClient client;
    while (true) {
        connectWifi();
        connectToDevice(client, IPAddress(192, 168, 0, 133));
        initRTC();

        //
        DebugLog(std::to_string(getUnixTime()));

        // parse from string and use it
        uint8_t ip[4] = {0,0,0,0}; ipv4_parse((uint8_t*)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);
        connectToDevice(client, IPAddress(ip));

        // tuya 3.4 testing
        uint8_t* to_send = (uint8_t*)calloc(1, 16); size_t len = 16;
        uint8_t* buffer = (uint8_t*)calloc(1, 256);
        uint8_t* recBuffer = (uint8_t*)calloc(1, 256); size_t recLen = 0;

        //
        size_t hmac_len = 48;
        size_t code_len = 0;
        uint8_t* hmac_key = (uint8_t*)calloc(1, 16);
        memcpy(hmac_key, tuya_local_key.c_str(), 16);

        // say hello with local_nonce with local_key encryption and checksum
        code_len = tc::encodeTuyaCode(tc::encryptDataECB((uint8_t*)(tuya_local_key.c_str()), (uint8_t*)tc::local_nonce, len, to_send, false), len, tc::TuyaCmd{1u, 0x3u, hmac_key}, buffer);
        waitAndSend(client, buffer, code_len); // send local nonce
        waitForReceive(client, recBuffer, recLen);

        DebugLog("Sent Code");
        if (code_len > 0) DebugCode(buffer, code_len);

        DebugLog("Received Code");
        if (recLen > 0) DebugCode(recBuffer, recLen);

        //
        //DebugLog(std::string((char*)buffer, code_len));

        // make linked with local remote hmac
        uint8_t* hmac = (uint8_t*)calloc(1, hmac_len);
        size_t remLength = 0; uint8_t* remote_nonce = tc::getTuyaPayload(recBuffer, remLength);
        tc::encode_remote_hmac((uint8_t*)(tuya_local_key.c_str()), remote_nonce, hmac);

        // send remote hmac
        code_len = tc::encodeTuyaCode(hmac, hmac_len, tc::TuyaCmd{2u, 0x5u, hmac_key}, buffer);
        waitAndSend(client, buffer, code_len);

        //DebugLog(std::string((char*)buffer, code_len));

        // encode hmac-based key (will be used for checksum)
        tc::encode_hmac_key((uint8_t*)(tuya_local_key.c_str()), remote_nonce, hmac_key);

        // debug any signal
        waitForReceive(client, recBuffer, recLen);

        DebugLog("Sent Code");
        if (code_len > 0) DebugCode(buffer, code_len);

        DebugLog("Received Code");
        if (recLen > 0) DebugCode(recBuffer, recLen);

        //
        /*ArduinoJson::JsonDocument doc;
        doc["t"] = getUnixTime();
        doc["devId"] = device_id;
        doc["uid"] = device_uuid;
        doc["gwId"] = device_id;*/


        //
        while (WiFi.status() == WL_CONNECTED) {
            M5.delay(1);


        }

        

    }
}

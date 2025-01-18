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
#include "handler/tuya_device.hpp"

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

        //
        /*ArduinoJson::JsonDocument doc;
        doc["t"] = getUnixTime();
        doc["devId"] = device_id;
        doc["uid"] = device_uuid;
        doc["gwId"] = device_id;*/

        //
        auto device = th::TuyaDevice34();
        while (WiFi.status() == WL_CONNECTED) {
            M5.delay(1);

            //
            if (!device.connected()) {
                device.connectDevice(tuya_local_ip, tuya_local_key, device_id, device_uuid);
                device.sendLocalNonce();

                // TODO: sending JSON
            }

            //
            device.handleSignal();
        }
    }
}

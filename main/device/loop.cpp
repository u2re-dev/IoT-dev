#define VERY_LARGE_STRING_LENGTH 8000
#define NTP_TIMEZONE  "UTC+7"

//

//
#include <WiFi.h>
#include <HardwareSerial.h>
#include <M5Unified.hpp>
#include <ArduinoJson.hpp>

//
#include "hal/interface.hpp"
#include "hal/network.hpp"
#include "m5stack/rtc.hpp"
#include "std/libtuya.hpp"

//
#include "../private.hpp"
#include "./std/std.hpp"
#include "./persistent/nv_typed.hpp"
#include "./handler/tuya_device.hpp"

//
void IOTask() {

};

//
void loop(void) {};
void setup(void)
{
    setCpuFrequencyMhz(160);

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nv::storage.begin("nvs", false);
    std::thread IOTaskThread = std::thread(IOTask);

    //
    bool once = false;

    //
    initWiFi();

    //
    while (true) {
        connectWifi();
        initRTC();

        //
        DebugLog(std::to_string(getUnixTime()));

        //
        ArduinoJson::JsonDocument dps;
        auto obj = dps.to<ArduinoJson::JsonObject>();
        obj["20"] = false;

        //
        auto device = th::TuyaDevice34();
        while (WiFi.status() == WL_CONNECTED) {
            M5.delay(1);

            //
            if (!device.connected()) {
                device.connectDevice(tuya_local_ip, tuya_local_key, device_id, device_uuid);
                device.sendLocalNonce();
            }

            //
            device.handleSignal();

            //
            if (device.available() && !once) {
                device.setDPS(obj); once = true;
            }
        }
    }
}

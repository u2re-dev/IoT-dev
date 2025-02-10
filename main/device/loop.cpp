#define VERY_LARGE_STRING_LENGTH 8000
#define NTP_TIMEZONE "UTC+7"

//
#ifdef USE_ARDUINO
#include <WiFi.h>
#include <HardwareSerial.h>
#endif

//
#ifdef USE_M5STACK
#include <M5Unified.hpp>
#endif

//
#ifdef USE_ARDUINO_JSON
#include <ArduinoJson.hpp>
#endif

//
#include "hal/interface.hpp"
#include "hal/network.hpp"
#include "m5stack/rtc.hpp"

//
#include "../private.hpp"
#include "./persistent/nv_typed.hpp"

//
#include "../tuya/libtuya.hpp"
#include "../std/std.hpp"

//
#include "./session/TuyaSession.hpp"

//
void IOTask() {

};

//
void loop(void) {};
void setup(void)
{
    setCpuFrequencyMhz(160);

    //
#ifdef USE_ARDUINO
    Serial.setDebugOutput(true);
    Serial.begin(115200);

    //
    nv::storage.begin("nvs", false);
#endif

    //
    std::thread IOTaskThread = std::thread(IOTask);

    //
    bool once = false;

    //
    initWiFi();

    //
    while (true)
    {
#ifdef USE_ARDUINO
        connectWifi();
#endif

        initRTC();

        //
        DebugLog(std::to_string(getUnixTime()));

        //
#ifdef USE_ARDUINO_JSON
        ArduinoJson::JsonDocument dps;
        auto obj = dps.to<ArduinoJson::JsonObject>();
        obj["20"] = false;
#endif

        //
        auto session = th::TuyaSession();

        //
#ifdef USE_ARDUINO
        while (WiFi.status() == WL_CONNECTED)
#endif
        {
#ifdef USE_M5STACK
            M5.delay(1);
#endif

            //
            if (!session.connected())
            {
                session.connectDevice(tuya_local_ip, tuya_local_key, device_id, device_uuid);
                session.sendLocalNonce();
            }

            //
            session.handleSignal();

            //
            if (session.available() && !once)
            {
#ifdef USE_ARDUINO_JSON
                device.setDPS(obj);
                once = true;
#endif
            }
        }
    }
}

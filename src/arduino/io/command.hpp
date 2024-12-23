#pragma once

//
#include <library/std.hpp>
#include <hal/interface/current.hpp>

//
#ifdef ENABLE_ARDUINO
#include "./fs.hpp"

//
const auto COMHandler = [](uint32_t command){
    if (command == 0x4) { device[CURRENT_DEVICE].tuyaInit(); } else
    if (command == 0x1) { switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2); } else
    if (command == 0x2) { switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1)); } else
    if (command == 0x3) 
    {
        switchScreen(true, CURRENT_DEVICE);

        //
        if (!fs::sd::loadConfig(FSHandler)) {
            if (!fs::internal::loadConfig(FSHandler)) {
                _STOP_EXCEPTION_();
            }
        }

        //
        while (LOADING_SD) {
            delay(1);
        }

        //
        if (!LOADING_SD) {
            Serial.println("Connecting to WiFi...");

            //
            WiFi.disconnect(true);
            WiFi.begin(wifi::ssid, wifi::password);

            //
            while (WiFi.status() != WL_CONNECTED)
            {
                delay(1);
            }
        }

        switchScreen(false, CURRENT_DEVICE);
    } else {
        Serial.println("Trying to send command...");
        //
        JSONVar _irMap_ = device[CURRENT_DEVICE].getIRMap();
        String cmdCode = String(command, HEX);
        if (_irMap_.hasOwnProperty(cmdCode)) {
            Serial.println("Sending command...");
            if (JSON.typeof(_irMap_[cmdCode]["20"]).startsWith("boolean")) {
                device[CURRENT_DEVICE].ENABLED = (bool)_irMap_[cmdCode]["20"];
            }
            device[CURRENT_DEVICE].sendControl(_irMap_[cmdCode]);
        }
    }
};
#endif

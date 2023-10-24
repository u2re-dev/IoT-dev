#pragma once

//
#include "../modules/http_server.hpp"
#include "../drivers/input/keypad_handle.hpp"
#include "../drivers/interface/current.hpp"
#include "./fs.hpp"

//
const auto COMHandler = [](uint32_t command){
    
    //
    if (command == 0x4) { device[CURRENT_DEVICE].tuyaInit(); } else
    if (command == 0x1) { switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2); } else
    if (command == 0x2) { switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1)); } else
    if (command == 0x3) 

    //
    //if (command == 0x43) { device[CURRENT_DEVICE].tuyaInit(); } else
    //if (command == 0x47) { switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2); } else
    //if (command == 0x45) { switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1)); } else
    //if (command == 0x9) 
    {
        switchScreen(true, CURRENT_DEVICE);

        //
        if (!loadConfigSD(FSHandler)) {
            if (!loadConfigInternal(FSHandler)) {
                _STOP_EXCEPTION_();
            }
        }

        //
        while (LOADING_SD) {
        #ifndef ESP32
            ui.update();
        #endif
            delay(1);
        }

        //
        if (!LOADING_SD) {
            Serial.println("Connecting to WiFi...");

            //
            WiFi.disconnect(true);
            WiFi.begin(ssid, password);

            //
        #ifndef ESP32
            while (!(WiFi.localIP().isSet() || WiFi.status() == WL_CONNECTED))
        #else
            while (WiFi.status() != WL_CONNECTED) 
        #endif
            {
        #ifndef ESP32
                ui.update();
        #endif
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

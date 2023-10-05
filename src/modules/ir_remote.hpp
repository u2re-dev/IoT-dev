#pragma once

//
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

//
#include "./sd_card.hpp"
#include "../tuya/tuya.hpp"
#include "../graphics/display.hpp"

//
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

//
static const uint16_t kRecvPin = 8;//27;
static IRrecv irrecv(kRecvPin);
static decode_results results;

//
void handleIR(TuyaDevice3* device) {
    if (irrecv.decode(&results)) {
        if (results.value != -1LL && results.command != 0) {
            // print() & println() can't handle printing long longs. (uint64_t)
            //Serial.println(String(results.command, HEX));
            //Serial.println("");
            
            //
            String cmdCode = String(results.command, HEX);
            _screen_[DEBUG_SCREEN ? 0 : (CURRENT_DEVICE+1)]._LINE_3_= "LastIR: " + cmdCode;

            //
            if (results.command == 0x43) {
              device[CURRENT_DEVICE].tuyaInit();
            } else

            //
            if (results.command == 0x47) {
              switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2);
            } else

            //
            if (results.command == 0x45) {
              switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1));
            } else

            //
            if (results.command == 0x9) {
                switchScreen(true, CURRENT_DEVICE);

                //
                reloadConfig(device);

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
                if (_irMap_.hasOwnProperty(cmdCode)) {
                    Serial.println("Sending command...");
                    if (JSON.typeof(_irMap_[cmdCode]["20"]).startsWith("boolean")) {
                        device[CURRENT_DEVICE].ENABLED = (bool)_irMap_[cmdCode]["20"];
                    }
                    device[CURRENT_DEVICE].sendControl(_irMap_[cmdCode]);
                }
            }
        }

        //
        results.command = 0;
        results.value = -1LL;

        //
        irrecv.resume();  // Receive the next value
    }
}

void initIR() {
    Serial.println("Enabling IR...");
    irrecv.enableIRIn();
}

#pragma once

//
#include "./sd_card.hpp"

//
#include <Wire.h>
//#include <WireSlaveRequest.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
//#include "graphics/display.hpp"

#define I2C_SDA 44
#define I2C_SCL 43
#define I2C_SLAVE_ADDR 0x32

//
#define MAX_B 1

uint8_t received_data[MAX_B] = {0};

//
void handleIR(TuyaDevice3* device) {
    //
    //WireSlaveRequest slaveReq(Wire, I2C_SLAVE_ADDR, MAX_B);
    Wire.requestFrom(0x32, MAX_B);

    // 
    for (int i = 0; i < MAX_B && Wire.available(); i++)
    {
        received_data[i] = Wire.read(); 
        //Serial.print(String(c, HEX));
        //Serial.print(" ");
    }
    //Serial.println("");

    //
    uint8_t command = received_data[0];

    //
    if (command) {
        // print() & println() can't handle printing long longs. (uint64_t)
        //Serial.println(String(results.command, HEX));
        //Serial.println("");

        //
        String cmdCode = String(command, HEX);
        _screen_[DEBUG_SCREEN ? 0 : (CURRENT_DEVICE+1)]._LINE_3_= "LastIR: " + cmdCode;

        //
        if (command == 0x4) {
            device[CURRENT_DEVICE].tuyaInit();
        } else

        //
        if (command == 0x1) {
            switchScreen(DEBUG_SCREEN, (CURRENT_DEVICE+1)%2);
        } else

        //
        if (command == 0x2) {
            switchScreen(DEBUG_SCREEN, CURRENT_DEVICE <= 0 ? 1 : (CURRENT_DEVICE-1));
        } else

        //
        if (command == 0x3) {
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

        //
        memset(received_data, 0, 64);
    }
}

void initIR() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("Slave joined I2C bus with addr #%d\n", I2C_SLAVE_ADDR);
}

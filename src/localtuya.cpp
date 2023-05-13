/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#define VERY_LARGE_STRING_LENGTH 8000

//
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>

//
#include "tuya.hpp"
#include "display.hpp"
#include "sd_card.hpp"
#include "ir_remote.hpp"
#include "http_server.hpp"

//
thread_local static JSONVar testProgramTemplate[3];
std::function<void(TuyaDevice3&)> testPrograms[] = {
    [](TuyaDevice3& device){
        JSONVar& _temp_ = testProgramTemplate[0];
        //device.merge(_temp_, device.cState);

        //
        //_temp_["21"] = String("colour");
        //_temp_["24"] = HSV_TO_HEX_B(float(getTime() % 360), 1.0, 0.1);

        //
        //_temp_["23"] = 1000 - ((getTime()*100) % 1000);
        //device.sendControl(_temp_);
    }
};

//
static TuyaDevice3 device[2] = { TuyaDevice3(&_screen_[1], "dev0"), TuyaDevice3(&_screen_[2], "dev1") };

//
void handleDevices() {
    if (WiFiConnected()) {
        for (uint8_t I=0;I<2;I++) {
            device[I].reconnect();
        }
        for (uint8_t I=0;I<2;I++) {
            device[I].handleReceive();
        }
        //for (uint8_t I=0;I<2;I++) {
            //device[I].handleAutoProgram();
        //}
    }
}

//
void setup() {
    //
    //EEPROM.begin(48);
    nvs.begin("nvs", false);

    //
    Serial.setDebugOutput(true);
    Serial.begin(115200);
    Wire.setClock(3400 * 1000);

    //
    initScreen();
    initDisplay();
    initRTC();
    initIR();

    //
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
    initWiFi();

    //
    while (!WiFiConnected())
    {
        handleIR(device);
#ifndef ESP32
        ui.update();
#endif
        delay(1);
    }

    //
    initServer(device);
    switchScreen(false, CURRENT_DEVICE);
    
    //
    Serial.println("Setup is done...");
}

//
void loop() {
    switchScreen((!CONNECTED.load() || LOADING_SD), CURRENT_DEVICE);
    handleIR(device);
    handleWiFi();

    //
    if (WiFiConnected()) {
        timeClient.update();
    }

    _syncTimeFn_();
    handleDevices();

#ifndef ESP32
    ui.update();
#endif
    delay(1);
}

#pragma once

//
#include "../imports/imports.hpp"
#include "../modules/tuya.hpp"

//
thread_local static JSONVar testProgramTemplate[3];
std::function<void(tuya::TuyaDevice3&)> testPrograms[] = {
    [](tuya::TuyaDevice3& device){
        JSONVar& _temp_ = testProgramTemplate[0];
    }
};

//
static tuya::TuyaDevice3 device[2] = { tuya::TuyaDevice3("dev0"), tuya::TuyaDevice3("dev1") };

//
void handleDevices() {
    if (wifi::WiFiConnected()) {
        device[CURRENT_DEVICE].reconnect();
        device[CURRENT_DEVICE].handleReceive();
    }
}

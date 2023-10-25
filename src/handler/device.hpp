#pragma once

//
#include "../modules/tuya.hpp"

//
thread_local static JSONVar testProgramTemplate[3];
std::function<void(tuya::TuyaDevice3&)> testPrograms[] = {
    [](tuya::TuyaDevice3& device){
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
static tuya::TuyaDevice3 device[2] = { tuya::TuyaDevice3("dev0"), tuya::TuyaDevice3("dev1") };

//
void handleDevices() {
    if (wifi::WiFiConnected()) {
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
#pragma once

//
#include <string>
#include <HardwareSerial.h>
#include <M5Unified.hpp>

//
std::string DebugLog(std::string message) {
    //Serial.println(message.c_str());
    M5.Log.println(message.c_str());
}

//
std::string DebugLine(std::string message) {
    //Serial.print(message.c_str());
    M5.Log.print(message.c_str());
}

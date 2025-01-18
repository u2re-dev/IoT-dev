#pragma once

//
#include <string>
#include <HardwareSerial.h>
#include <M5Unified.hpp>

//
std::string DebugLog(std::string message) {
    //Serial.println(message.c_str());
    M5.Log.println(message.c_str());
    return message;
}

//
std::string DebugLine(std::string message) {
    //Serial.print(message.c_str());
    M5.Log.print(message.c_str());
    return message;
}

//
static unsigned long lastTime = 0;
std::string DebugLineWithInterval(std::string message, unsigned long interval = 100) {
    if (!lastTime) { lastTime = millis(); };
    //Serial.print(message.c_str());
    if ((millis() - lastTime) >= interval) {
        M5.Log.print(message.c_str());
        lastTime = millis();
    };
    return message;
}

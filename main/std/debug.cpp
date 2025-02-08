//
#include <string>
#include <HardwareSerial.h>
#include <M5Unified.hpp>
#include <Arduino.h>

//
#include "./debug.hpp"

//
static unsigned long lastTime = 0;

//
std::string DebugLog(std::string message) {
    M5.Log.println(message.c_str());
    return message;
}

//
std::string DebugLine(std::string message) {
    M5.Log.print(message.c_str());
    return message;
}

//
std::string DebugLineWithInterval(std::string message, unsigned long interval) {
    if (!lastTime) { lastTime = millis(); };
    //Serial.print(message.c_str());
    if ((millis() - lastTime) >= interval) {
        M5.Log.print(message.c_str());
        lastTime = millis();
    };
    return message;
}

//
std::string DebugCode(uint8_t const* code, size_t length) {
    Serial.print("0x");
    for (uint i=0;i<length;i++) {
        if (code[i] < 16) { Serial.print("0"); };
        Serial.print(code[i], HEX);
    }
    M5.Log.println("");
    return "";
}

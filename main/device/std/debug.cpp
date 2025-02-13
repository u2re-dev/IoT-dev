//
#include <string>
#include <HardwareSerial.h>
#include <Arduino.h>

//
#ifdef USE_M5STACK
#include <M5Unified.hpp>
#endif

//
#include "./debug.hpp"

//
static unsigned long lastTime = 0;

//
std::string DebugLog(std::string message) {
#ifdef USE_M5STACK
    M5.Log.println(message.c_str());
#endif
    return message;
}

//
std::string DebugLine(std::string message) {
#ifdef USE_M5STACK
    M5.Log.print(message.c_str());
#endif
    return message;
}

//
std::string DebugLineWithInterval(std::string message, unsigned long interval) {
    if (!lastTime) { lastTime = millis(); };
    //Serial.print(message.c_str());
    if ((millis() - lastTime) >= interval) {
#ifdef USE_M5STACK
        M5.Log.print(message.c_str());
#endif
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
#ifdef USE_M5STACK
    M5.Log.println("");
#endif
    return "";
}

#pragma once
#include <M5Unified.hpp>

//
#define NTP_TIMEZONE  "JST-9"
#define NTP_SERVER1   "0.pool.ntp.org"
#define NTP_SERVER2   "1.pool.ntp.org"
#define NTP_SERVER3   "2.pool.ntp.org"

//
#if __has_include (<esp_sntp.h>)
    #include <esp_sntp.h>
    #define SNTP_ENABLED 1
#elif __has_include (<sntp.h>)
    #include <sntp.h>
    #define SNTP_ENABLED 1
#endif
#include <WiFi.h>
#include <std/debug.hpp>

//
#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

//
void initRTC() {
    M5.setLogDisplayIndex(0);
    configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    //
    struct tm timeInfo;
    const auto start = millis();
    auto status = getLocalTime(&timeInfo, 1000);
    while (!status) { 
        DebugLine(".");
        if ((millis() - start) > 1000 || WiFi.status() != WL_CONNECTED) break;
        status = getLocalTime(&timeInfo, 1000);
    };
    DebugLog("");

    //
    if (status) {
        time_t t = time(nullptr)+1;
        while (t > time(nullptr));
        M5.Rtc.setDateTime(gmtime(&t));
        DebugLog("Setting RTC done");
    } else {
        DebugLog("Setting RTC failed");
    }
}

//
time_t getUnixTime() {
    // ESP32 internal timer
    auto t = time(nullptr);
    gmtime(&t); // UTC
    //localtime(&t);
    return t;
}

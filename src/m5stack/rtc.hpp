#pragma once
#include <M5Unified.hpp>
#include <WiFi.h>
#include <std/debug.hpp>

//
#if __has_include (<esp_sntp.h>)
    #include <esp_sntp.h>
    #define SNTP_ENABLED 1
#elif __has_include (<sntp.h>)
    #include <sntp.h>
    #define SNTP_ENABLED 1
#endif

//
const long  gmtOffset_sec = 3600 * 7;
const int   daylightOffset_sec = 0;

//
void initRTC() {
    M5.setLogDisplayIndex(0);

    //
    sntp_set_sync_interval(60000);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "1.pool.ntp.org");
    sntp_setservername(2, "2.pool.ntp.org");
    sntp_init();

    // restart sntp
    if (sntp_enabled()) { sntp_stop(); }
    sntp_init();

    // set timezone
    setenv("TZ", "UTC+7", 1);
    tzset();

    //
    const auto start = millis();
    auto status = sntp_get_sync_status();
    while (status != SNTP_SYNC_STATUS_COMPLETED) {
        status = sntp_get_sync_status();
        DebugLineWithInterval(".", 100);
        if ((millis() - start) > 10000) break;
    }
    DebugLog("");

    //
    if (status != SNTP_SYNC_STATUS_COMPLETED) {
        DebugLog("Sync with NTP was failed");
    }

    //
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        time_t t = time(nullptr);
        DebugLog("Getting local time done");
        M5.Rtc.setDateTime(gmtime(&t));
    } else {
        DebugLog("Getting local time failed");
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

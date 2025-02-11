#include "./rtc.hpp"

//
#ifdef USE_M5STACK_RTC
#include <M5Unified.hpp>
#endif

//
#ifdef USE_ARDUINO
#include <WiFi.h>
#endif

//
#include "../../std/debug.hpp"

//
const long  gmtOffset_sec = 3600 * 7;
const int   daylightOffset_sec = 0;

//
#ifndef USE_ARDUINO
inline uintptr_t millis() { return 1; };
#endif

//
void initRTC() {
    //M5.setLogDisplayIndex(0);

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
#ifdef USE_ARDUINO
    if (getLocalTime(&timeInfo))
#endif
    {
        time_t t = time(nullptr);
        DebugLog("Getting local time done");
#ifdef USE_M5STACK_RTC
        M5.Rtc.setDateTime(gmtime(&t));
#endif
    }
#ifdef USE_ARDUINO
    else {  DebugLog("Getting local time failed"); }
#endif
}

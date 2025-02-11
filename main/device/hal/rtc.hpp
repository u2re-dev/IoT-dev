#pragma once

//
#if __has_include (<esp_sntp.h>)
    #include <esp_sntp.h>
    #define SNTP_ENABLED 1
#elif __has_include (<sntp.h>)
    #include <sntp.h>
    #define SNTP_ENABLED 1
#endif

//
void initRTC();

//
inline time_t getUnixTime() {
    // ESP32 internal timer
    auto t = time(nullptr);
    gmtime(&t); // UTC
    //localtime(&t);
    return t;
}

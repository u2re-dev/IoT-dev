#pragma once

//
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

//
#ifdef ESP32
#include <ESP32Time.h>
#endif

//
#define ENABLE_DS1307

//
#ifdef ENABLE_DS1307
#include <RTCx.h>
static bool DS1307_INITIALIZED = false;
#endif

//
#include <NTPClient.h>
#include <Time.h>
#include "timed.hpp"

//
//WiFiUDP client;
static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, "pool.ntp.org");
static const uint64_t unix_shift = 946684800;


//
/*void setInternalTime(uint64_t epoch = 0, uint32_t us = 0)
{
  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = us;
  settimeofday(&tv, NULL);
}*/

//
int64_t getTimestamp(int year, int mon, int mday, int hour, int min, int sec)
{
    const uint16_t ytd[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; /* Anzahl der Tage seit Jahresanfang ohne Tage des aktuellen Monats und ohne Schalttag */
    int leapyears = ((year - 1) - 1968) / 4
                    - ((year - 1) - 1900) / 100
                    + ((year - 1) - 1600) / 400; /* Anzahl der Schaltjahre seit 1970 (ohne das evtl. laufende Schaltjahr) */
    int64_t days_since_1970 = (year - 1970) * 365 + leapyears + ytd[mon - 1] + mday - 1;
    if ( (mon > 2) && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ) days_since_1970 += 1; /* +Schalttag, wenn Jahr Schaltjahr ist */
    return sec + 60 * (min + 60 * (hour + 24 * days_since_1970) );
}

// Function that gets current epoch time
/*unsigned long getTime() {
  timeClient.update();
  return timeClient.getEpochTime();
}*/

//
#ifdef ENABLE_DS1307
static RTCx ds1307;

//
time_t ds1307_getEpoch() {
    tm _tm_ = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//#ifdef ESP32
    //uint32_t volatile register ilevel = XTOS_DISABLE_ALL_INTERRUPTS;
//#endif
    if (DS1307_INITIALIZED) {
        ds1307.readClock((tm*)&_tm_);
    }
//#ifdef ESP32
    //XTOS_RESTORE_INTLEVEL(ilevel);
//#endif
    return RTCx::mktime((tm*)&_tm_);
}

void ds1307_setEpoch(time_t epoch) {
    tm _tm_ = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    gmtime_r(&epoch, (tm*)&_tm_);
//#ifdef ESP32
    //uint32_t volatile register ilevel = XTOS_DISABLE_ALL_INTERRUPTS;
//#endif
    if (DS1307_INITIALIZED) {
        ds1307.setClock((tm*)&_tm_);
    }
//#ifdef ESP32
    //XTOS_RESTORE_INTLEVEL(ilevel);
//#endif
}
#endif

//
#ifdef ESP32
static ESP32Time rtc;
#endif

//std::atomic<unsigned long> _last_sync_time_;

//
void _syncTime_() {
    // set correct timezone
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.begin();
        timeClient.setTimeOffset(3600 * 7);
    }

    //
    Serial.println("Syncing time...");

    //
    if (WiFi.status() == WL_CONNECTED && (timeClient.isTimeSet() || timeClient.update())) {
        Serial.println("Through NTP...");
#ifdef ENABLE_DS1307
        ds1307_setEpoch(timeClient.getEpochTime());
#endif
#ifdef ESP32
//#ifdef ENABLE_DS1307
        // FOR TEST AND DEBUG RTC ONLY!
        //rtc.setTime(ds1307_getEpoch() ? ds1307_getEpoch() : timeClient.getEpochTime());
//#else
        // Real Perduino!
        rtc.setTime(timeClient.getEpochTime());
//#endif
#endif
    }
#ifdef defined(ENABLE_DS1307) && defined(ESP32)
    else if (ds1307_getEpoch()) {
        Serial.println("Through RTC...");
        rtc.setTime(ds1307_getEpoch());
    }
#endif
}


//
time_t _getTime_() {


    Serial.println("Getting time...");
#ifdef ESP32
    {
        Serial.println("Through Internal...");
        return time_t(rtc.getEpoch());
    }
#else

    if (WiFi.status() == WL_CONNECTED && (timeClient.update() || timeClient.isTimeSet())) {
        Serial.println("Through NTP...");
        return time_t(timeClient.getEpochTime());
    } 
#ifdef ENABLE_DS1307
    else if (ds1307_getEpoch()) {
        Serial.println("Through DS1307...");
        return ds1307_getEpoch();
    }
#endif

#endif
}

// Convert compile time to system time 
time_t cvt_date(char const *date, char const *time)
{
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(time, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    // Find where is s_month in month_names. Deduce month value.
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3 + 1;    
    t.tm_year = year - 1900;    
    return mktime(&t);
}


//
static std::atomic<unsigned long> _current_time_;
static IntervalFn<15000> _syncTimeFn_([&](){
    _current_time_ = _getTime_();
});

//
unsigned long getTime() {
    return _current_time_ + (_syncTimeFn_.millisSince()/1000);
}


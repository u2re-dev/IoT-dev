#pragma once

//
#include <Wire.h>
#include <NTPClient.h>
#include <ESP32Time.h>

//
#include "../network/wifi.hpp"
#include "./timer.hpp"

//
namespace rtc {

    //
    static WiFiUDP ntpUDP;
    static NTPClient timeClient(ntpUDP, "pool.ntp.org");
    static const uint64_t unix_shift = 946684800;

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

    static ESP32Time rtc;

    //
    void _syncTime_() {
        // set correct timezone
        if (wifi::WiFiConnected()) {
            timeClient.begin();
            timeClient.setTimeOffset(3600 * 7);
        }

        //
        Serial.println("Syncing time...");

        //
        if (wifi::WiFiConnected() && (timeClient.isTimeSet() || timeClient.update())) {
            Serial.println("Through NTP...");
            rtc.setTime(timeClient.getEpochTime());
        }
    }


    //
    time_t _getTime_() {
        Serial.println("Getting time...");
        {
            Serial.println("Through Internal...");
            return time_t(rtc.getEpoch());
        }

        if (WiFiConnected() && (timeClient.update() || timeClient.isTimeSet())) {
            Serial.println("Through NTP...");
            return time_t(timeClient.getEpochTime());
        } 
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
    static timer::IntervalFn<15000> _syncTimeFn_([](){
        _current_time_ = _getTime_();
    });

    //
    unsigned long getTime() {
        return _current_time_ + (_syncTimeFn_.millisSince()/1000);
    }

    //
    void initRTC() {
        time_t compiled = cvt_date(__DATE__, __TIME__);
        Serial.println("Compiled Epoch: " + String(compiled));

        //
        time_t inow = rtc.getEpoch();
        Serial.println("ESP32 RTC Epoch: " + String(inow));
        if (icom > inow) {
            Serial.println("Internal RTC has wrong time, reset...");
            rtc.setTime(icom);
        }

        //
        _syncTimeFn_();
    }
}
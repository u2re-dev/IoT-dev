#pragma once

//
#include "wifi.hpp"
#include "screen.hpp"
#include "tuya.hpp"

//#include <FS.h>
#include <SD.h>

//
static nv_bool LOADING_SD;

//
bool reloadConfig(TuyaDevice3* device) {
    bool LOADED = false;
    //LOADING_SD = true;

//
static const char *filename = "/keys.json";

//
#ifdef ESP32
static const int CS_PIN = 10;//5;
#else
static const int CS_PIN = 15;
#endif

    //
    Serial.println("Reading from SD...");
    _screen_[0]._LINE_1_ = "Please, insert SD card with...";
    _screen_[0]._LINE_2_ = filename;

    //
    //bool loaded = false;
    if (SD.begin(CS_PIN)) {
        // Open file for writing
        Serial.println("SD connected...");
        File file = SD.open(filename, FILE_READ);
        if (!file) {
          Serial.println("Failed to read file, HALTED!");
          _screen_[0]._LINE_1_ = "Failed to read file, HALTED!";
          return false;
        }

        //
        JSONVar doc = JSON.parse(file.readString());
        if (JSON.typeof(doc) == "undefined") {
          Serial.println(F("Failed to read file, using default configuration"));
          _screen_[0]._LINE_1_ = "Wrong file, HALTED!";
          return false;
        }
        
        //
        delay(10);
        file.close();

        //
        Serial.println("Success to read JSON file...");
        _screen_[0]._LINE_1_ = "Success to read JSON file...";
        _screen_[0]._LINE_2_ = "";
        delay(10);

        //
        ssid = (char const*)doc["ssid"];
        password = (char const*)doc["password"];

        Serial.println("SSID: " + ssid.toString());
        Serial.println("Password: " + password.toString());

        //
        delay(10);

        //
        Serial.println("Configuring devices...");
        _screen_[0]._LINE_1_ = "Configuring devices...";

        //
        device[0].loadConfig(doc["devices"][0]);
        device[1].loadConfig(doc["devices"][1]);

        

        // eenforce enabled state
        /*
        device[0].setAutoProgram([](TuyaDevice3& _device_){
            JSONVar _tmp_;
            _tmp_["20"] = _device_.ENABLED;
            _device_.sendControl(_tmp_);
        }, 2000);
        device[1].setAutoProgram([](TuyaDevice3& _device_){
            JSONVar _tmp_;
            _tmp_["20"] = _device_.ENABLED;
            _device_.sendControl(_tmp_);
        }, 2000);*/

        //device[0].setAutoProgram(testPrograms[0], 2000);

        //
        delay(10);

        //
        Serial.println("Configured...");
        _screen_[0]._LINE_1_ = "Configured...";

        //
        LOADING_SD = false;
        LOADED = true;
    }

    //
    return LOADED;
}

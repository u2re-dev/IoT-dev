#pragma once

//
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

//
#include "../interface/current.hpp"
#include "../persistent/nv_typed.hpp"

//
bool loadConfigInternal(std::function<void(JSONVar&)> handler) {
    //
    LOADING_SD = false;
    bool LOADED = false;

    //
    static const char *filename = "/keys.json";
    Serial.println("Reading from Internal Storage...");
    _LOG_(1, filename);

    //
    if (SPIFFS.begin()) {
        LOADING_SD = true;
        // Open file for writing
        Serial.println("SD connected...");
        File file = SPIFFS.open(filename, FILE_READ);
        if (!file) {
          Serial.println("Failed to read file, HALTED!");
          _LOG_(0, "Failed to read file, HALTED!");
          return (LOADING_SD = false);
        }

        //
        JSONVar doc = JSON.parse(file.readString());
        if (JSON.typeof(doc) == "undefined") {
          Serial.println(F("Failed to read file, using default configuration"));
          _LOG_(0, "Wrong file, HALTED!");
          return (LOADING_SD = false);
        }
        
        //
        delay(10);
        file.close();

        //
        Serial.println("Success to read JSON file...");
        _LOG_(0, "Success to read JSON file...");
        _LOG_(1, "");
        delay(10);

        //
        Serial.println("Configuring devices...");
        _LOG_(0, "Configuring devices...");

        //
        handler(doc);

        //
        delay(10);

        //
        Serial.println("Configured...");
        _LOG_(0, "Configured...");

        //
        LOADING_SD = false;
        LOADED = true;
    }

    //
    return LOADED;
}

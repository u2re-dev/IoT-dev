#pragma once

//
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

//
#include "../interface/current.hpp"
#include "../persistent/nv_typed.hpp"

//
bool loadConfigSD(std::function<void(JSONVar&)> handler) {
    //
    LOADING_SD = false;
    bool LOADED = false;

    // Initialize SPI bus for microSD Card
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    digitalRead(SD_CS);

    //
    static const char *filename = "/keys.json";

    //
    Serial.println("Reading from SD...");
    _LOG_(0, "Please, insert SD card with...");
    _LOG_(1, filename);

    //
    SPIClass SPI0(HSPI);
    SPI0.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
    if (SD.begin(SD_CS)) {
        LOADING_SD = true;
        // Open file for writing
        Serial.println("SD connected...");
        File file = SD.open(filename, FILE_READ);
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
        ssid = (char const*)doc["ssid"];
        password = (char const*)doc["password"];

        Serial.println("SSID: " + ssid.toString());
        Serial.println("Password: " + password.toString());

        //
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

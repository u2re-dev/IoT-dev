#pragma once

//
#include <hal/core/io.hpp>
#include <hal/interface/current.hpp>
#include <core/persistent/nv_typed.hpp>

//
namespace fs {

    namespace sd {
        //
        bool loadConfig(std::function<void(JSONVar&)> handler) {
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
            _LOG_(0, "Reading from SD card...");
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
                  _LOG_(0, "Failed to read file...!");
                  return (LOADING_SD = false);
                }

                //
                JSONVar doc = JSON.parse(file.readString());
                if (JSON.typeof(doc) == "undefined") {
                  _LOG_(0, "Failed to read file...");
                  return (LOADING_SD = false);
                }
                
                //
                file.close();

                //
                _LOG_(0, "Success to read JSON file...");
                _LOG_(1, "");

                //
                handler(doc);

                //
                LOADING_SD = false;
                LOADED = true;
            }

            //
            return LOADED;
        }
    }

};

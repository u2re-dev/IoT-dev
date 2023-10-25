#pragma once

//
#include <SPIFFS.h>

//
#include "../interface/current.hpp"
#include "../persistent/nv_typed.hpp"

//
namespace fs {

    //
    namespace internal {

        //
        bool loadConfig(std::function<void(JSONVar&)> handler) {
            //
            LOADING_SD = false;
            bool LOADED = false;

            //
            static const char *filename = "/keys.json";

            //
            _LOG_(0, "Reading from Internal Storage...");
            _LOG_(1, filename);

            //
            if (SPIFFS.begin()) {
                LOADING_SD = true;
                // Open file for writing
                Serial.println("SD connected...");
                File file = SPIFFS.open(filename, FILE_READ);
                if (!file) {
                  _LOG_(0, "Failed to read file...");
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
                Serial.println("Success to read JSON file...");
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
}
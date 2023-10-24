#pragma once

//
#include <ESPAsyncWebServer.h>

//
#include "./tuya.hpp"

//
static AsyncWebServer server(80);

//
void initServer(TuyaDevice3* device) {
    //
    Serial.println("Setting protocol server...");

    //
    server.on("/", HTTP_GET, [device](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [device] (AsyncWebServerRequest *request) {

        //
        JSONVar _var_; uint8_t where = 0;
        if (request->hasParam("device")) {
            where = max(min(int(request->getParam("device")->value().toInt()), 1), 0);
        }

        //
        JSONVar _dps_;
        if (request->hasParam("20")) {
            // Perduino!
            String _val_ = request->getParam("20")->value(); _val_.trim();
            if (_val_ == "true") {
                _dps_["20"] = true;
            } else 
            if (_val_ == "false") {
                _dps_["20"] = false;
            } else {
                _dps_["20"] = bool(_val_.toInt());
            }
        }

        //
        if (request->hasParam("21")) { _dps_["21"] = request->getParam("21")->value(); }
        if (request->hasParam("22")) { _dps_["22"] = uint(request->getParam("22")->value().toInt()); }
        if (request->hasParam("23")) { _dps_["23"] = uint(request->getParam("23")->value().toInt()); }

        //
        if (where < 2 && where >= 0) {
            if (request->hasParam("reconnect")) { device[where].tuyaInit(); } 
            if (request->hasParam("20") || request->hasParam("21") || request->hasParam("22") || request->hasParam("23")) {
                if (request->hasParam("20")) {
                    device[where].ENABLED = (bool)_dps_["20"];
                }
                device[where].sendControl(_dps_);
            }
            request->send(200, "text/plain", JSON.stringify(device[where].cState));
        } else {
            request->send(200, "text/plain", "{}");
        }
    });

    //
    server.on("/post", HTTP_POST, [device](AsyncWebServerRequest *request) {
        /*String message;
        if (request->hasParam("message", true)) {
            message = request->getParam("message", true)->value();
        } else {
            message = "No message sent";
        }
        Serial.println(message);
        request->send(200, "text/plain", "Hello, POST: " + message);*/
        request->send(200, "text/plain", "Hello, POST: TODO");
    });

    //
    server.onNotFound([device](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });

    //
    server.begin();
}

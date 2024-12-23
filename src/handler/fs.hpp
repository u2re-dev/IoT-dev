#pragma once

//
#include <library/std.hpp>

//
#include <hal/fs/sd_card.hpp>
#include <hal/fs/internal.hpp>
#include <hal/network/wifi.hpp>

//
#include "./device.hpp"

//
const auto FSHandler = [](JSONVar& doc){
    //
    Serial.println("SSID: " + (wifi::ssid = (char const*)doc["ssid"]).toString());
    Serial.println("Password: " + (wifi::password = (char const*)doc["password"]).toString());

    //
    _LOG_(0, "Configuring devices...");
    device[0].loadConfig(doc["devices"][0]);
    device[1].loadConfig(doc["devices"][1]);
    _LOG_(0, "Configured...");
};

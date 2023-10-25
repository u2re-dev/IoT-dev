#pragma once

//
#include "../core/fs/sd_card.hpp"
#include "../core/fs/internal.hpp"
#include "../core/network/wifi.hpp"
#include "../handler/device.hpp"

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

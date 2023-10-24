#pragma once

//
#include "../drivers/fs/sd_card.hpp"
#include "../drivers/fs/internal.hpp"
#include "../drivers/network/wifi.hpp"
#include "../handler/device.hpp"

//
const auto FSHandler = [](JSONVar& doc){
    //
    Serial.println("SSID: " + (ssid = (char const*)doc["ssid"]).toString());
    Serial.println("Password: " + (password = (char const*)doc["password"]).toString());

    //
    device[0].loadConfig(doc["devices"][0]);
    device[1].loadConfig(doc["devices"][1]);
};

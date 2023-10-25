#pragma once

//
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>

//
#include "../interface/current.hpp"
#include "../interface/pin_config.hpp"
#include "../interface/i2c.hpp"

// Max buffer size
#define MAX_B 1

//
namespace keypad {

    //
    void handleInput() {
        //
        static uint8_t received_data[MAX_B] = {0};
        Wire.requestFrom(I2C_KEYPAD_ADDR, MAX_B);
        for (int i = 0; i < MAX_B && Wire.available(); i++)
        {   received_data[i] = Wire.read(); }

        //
        uint8_t command = received_data[0];
        if (command) {
            _LOG_(2, "Last Key: " + command);
            COM_HANDLER(command);
            memset(received_data, 0, MAX_B);
        }
    }

    //
    void initInput(std::function<void(uint32_t)> $) {
        Wire.begin(I2C_SDA, I2C_SCL);
        COM_HANDLER = $;
    }

}

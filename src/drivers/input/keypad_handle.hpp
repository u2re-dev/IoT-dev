#pragma once

//
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

//
#include "../../interface/current.hpp"

//
#define I2C_SDA 44
#define I2C_SCL 43
#define I2C_SLAVE_ADDR 0x32

//
#define MAX_B 1

//
uint8_t received_data[MAX_B] = {0};
std::function<void(uint32_t)> handler;

//
void handleIR() {
    Wire.requestFrom(0x32, MAX_B);

    // 
    for (int i = 0; i < MAX_B && Wire.available(); i++)
    {
        received_data[i] = Wire.read();
    }
    
    //
    uint8_t command = received_data[0];
    if (command) {
        _LOG_(2, "Last Key: " + command);
        handler(command);
        memset(received_data, 0, 64);
    }
}

void initIR(std::function<void(uint32_t)> $) {
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("Slave joined I2C bus with addr #%d\n", I2C_SLAVE_ADDR);
    handler = $;
}

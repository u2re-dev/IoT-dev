#pragma once

//
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

//
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

//
#include "../../interface/current.hpp"

//
static const uint16_t kRecvPin = 3;
static IRrecv irrecv(kRecvPin);
static decode_results results;
std::function<void(uint32_t)> handler;

//
void handleIR(std::function<void(uint32_t)> handler) {
    if (irrecv.decode(&results)) {
        if (results.value != -1LL && results.command != 0) {
            _LOG_(2, "Last IR: " + String(results.command, HEX));
            handler(results.command);
        }

        //
        results.command = 0;
        results.value = -1LL;
        irrecv.resume();  // Receive the next value
    }
}

void initIR() {
    Serial.println("Enabling IR...");
    irrecv.enableIRIn();
}

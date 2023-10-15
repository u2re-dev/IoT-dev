#pragma once

//
#ifdef ESP32
#include <thread>
//#include <SimplyAtomic.h>
#endif

//
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "pin_config.hpp"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif

//
#include "./screen.hpp"
#include "../time/rtc.hpp"

//
TFT_eSPI tft = TFT_eSPI();

//
void switchScreen(bool dbg, uint dvID) {
    if (DEBUG_SCREEN != dbg || CURRENT_DEVICE != dvID) {
        DEBUG_SCREEN = dbg;
        CURRENT_DEVICE = std::max(std::min(dvID, 1u), 0u);
    }
}

//
void msOverlay(TFT_eSPI *display) {
    display->setTextColor(TFT_BLACK, TFT_WHITE, true);
    time_t _time_ = getTime();
    display->drawRightString(String((_time_/3600)%24) + ":" + String((_time_/60)%60) + ":" + String(_time_%60), 0, 0, 2);
    display->drawString(DEBUG_SCREEN ? "Debug" : ("Device: " + String(CURRENT_DEVICE)), 0, 0, 2);
}

//
void _drawScreen_(TFT_eSPI *display, int16_t x, int16_t y, uint SCREEN_ID) {
    display->setTextColor(TFT_BLACK, TFT_WHITE, true);
    display->drawString(_screen_[SCREEN_ID]._LINE_1_.toString(), 0 + x, 11 + y, 2);
    display->drawString(_screen_[SCREEN_ID]._LINE_2_.toString(), 0 + x, 22 + y, 2);
    display->drawString(_screen_[SCREEN_ID]._LINE_3_.toString(), 0 + x, 33 + y, 2);
}

//
#ifdef ESP32
std::thread displayTask;
void displayThread() {
    while(true) {
        _drawScreen_(&tft, 0, 0, DEBUG_SCREEN ? 0 : max(min(CURRENT_DEVICE+1, 2u), 1u));
        msOverlay(&tft);
        delay(20);
    }
}
#endif

//
void initDisplay(void)
{
    Serial.begin(115200);
    Serial.println("Init Display...");

    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_WHITE);

    //
#ifdef ESP32
    Serial.println("Pinning to Core...");
    displayTask = std::thread(displayThread);
#endif
}

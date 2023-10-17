#pragma once

//
#ifdef ESP32
#include <thread>
//#include <SimplyAtomic.h>
#endif

//
#include "pin_config.hpp"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

//
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
    display->setTextColor(TFT_WHITE, TFT_BLACK, true);
    time_t _time_ = getTime();

    display->fillRect(10, 10, 320 - 20, 20, TFT_BLACK);
    display->drawRightString(String((_time_/3600)%24) + ":" + String((_time_/60)%60) + ":" + String(_time_%60), 310, 10, 2);
    display->drawString(DEBUG_SCREEN ? "Debug" : ("Device: " + String(CURRENT_DEVICE)), 10, 10, 2);
}

//
void _drawScreen_(TFT_eSPI *display, int16_t x, int16_t y, uint SCREEN_ID) {
    display->setTextColor(TFT_WHITE, TFT_BLACK, true);

    display->fillRect(10, 10 + 20, 320 - 20, 20, TFT_BLACK);
    display->drawString(_screen_[SCREEN_ID]._LINE_1_.toString(), 10 + x, 10 + 20 + y, 2);

    display->fillRect(10, 10 + 40, 320 - 20, 20, TFT_BLACK);
    display->drawString(_screen_[SCREEN_ID]._LINE_2_.toString(), 10 + x, 10 + 40 + y, 2);

    display->fillRect(10, 10 + 60, 320 - 20, 20, TFT_BLACK);
    display->drawString(_screen_[SCREEN_ID]._LINE_3_.toString(), 10 + x, 10 + 60 + y, 2);
}

//
static thread_local size_t lT = millis();

//
#ifdef ESP32
std::thread displayTask;
void displayThread() {
    while(true) {
        if ((millis() - lT) >= 100) {
            _drawScreen_(&tft, 0, 0, DEBUG_SCREEN ? 0 : max(min(CURRENT_DEVICE+1, 2u), 1u));
            msOverlay(&tft);
            lT = millis();
        }
        delay(1);
    }
}
#endif

//
void initDisplay(void)
{
    Serial.begin(115200);
    Serial.println("Init Display...");

    //
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);
    delay(100);

    //
    digitalWrite(PIN_POWER_ON, HIGH);
    digitalWrite(PIN_LCD_BL, HIGH);
    delay(100);
    
    //
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    //
#ifdef ESP32
    Serial.println("Pinning to Core...");
    displayTask = std::thread(displayThread);
#endif
}

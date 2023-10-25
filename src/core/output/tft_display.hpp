#pragma once

//
#ifdef ESP32
#include <thread>
#endif

//
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

//
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif

//
#include "../time/rtc.hpp"
#include "../interface/pin_config.hpp"
#include "../interface/current.hpp"

//
namespace tft {

    //
    TFT_eSPI display = TFT_eSPI();

    //
    void msOverlay(TFT_eSPI *display) {
        display->setTextColor(TFT_WHITE, BG_COLOR, true);
        time_t _time_ = rtc::getTime();

        display->fillRect(10, 10, 320 - 20, 20, BG_COLOR);
        display->drawRightString(String((_time_/3600)%24) + ":" + String((_time_/60)%60) + ":" + String(_time_%60), 310, 10, 2);
        display->drawString(DEBUG_SCREEN ? "Debug" : ("Device: " + String(CURRENT_DEVICE)), 10, 10, 2);
    }

    //
    void _drawScreen_(TFT_eSPI *display, int16_t x, int16_t y, uint SCREEN_ID) {
        display->setTextColor(TFT_WHITE, BG_COLOR, true);

        display->fillRect(10, 10 + 20, 320 - 20, 20, BG_COLOR);
        display->drawString(debug_info._LINE_[0].toString(), 10 + x, 10 + 20 + y, 2);

        display->fillRect(10, 10 + 40, 320 - 20, 20, BG_COLOR);
        display->drawString(debug_info._LINE_[1].toString(), 10 + x, 10 + 40 + y, 2);

        display->fillRect(10, 10 + 60, 320 - 20, 20, BG_COLOR);
        display->drawString(debug_info._LINE_[2].toString(), 10 + x, 10 + 60 + y, 2);
    }

    //
    static thread_local size_t lT = millis();

    //
    #ifdef ESP32
    std::thread displayTask;
    void displayThread() {
        while(true) {
            if ((millis() - lT) >= 100) {
                display.fillScreen(BG_COLOR);
                _drawScreen_(&display, 0, 0, DEBUG_SCREEN ? 0 : max(min(CURRENT_DEVICE+1, 2u), 1u));
                msOverlay(&display);
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
        display.init();
        display.setRotation(3);
        display.fillScreen(BG_COLOR);

        //
    #ifdef ESP32
        Serial.println("Pinning to Core...");
        displayTask = std::thread(displayThread);
    #endif
    }

}
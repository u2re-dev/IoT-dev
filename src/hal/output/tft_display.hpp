#pragma once

//
#include <library/std.hpp>
#include <hal/core/display.hpp>
#include <hal/network/rtc.hpp>
#include <hal/interface/current.hpp>

//
namespace tft {

    //
    TFT_eSPI display = TFT_eSPI();

    //
    void _RSOD_() {
        // do RSOD crash screen
        if (!POWER_SAVING.load()) {
            display.fillScreen(BG_COLOR = 0xF800);
            display.drawString("Our system run into problem, ", 10, 10);
            display.drawString("Needs to be a restart...", 10, 30);
            display.drawString("EXCEPTION CODE: " + String(EXCEPTION.load(), HEX), 10, 50);
        }
    }

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
    void initDisplay(void)
    {
        Serial.println("Init Display...");
        display.init();
        display.setRotation(1);
        display.fillScreen(BG_COLOR);
    }

    //
    void drawFrame() {
        //
        static thread_local size_t lT = millis();
        static thread_local size_t oT = millis();

        //
        if (!POWER_SAVING.load()) {
            
            //
            if (SHOW_CHANGED.load() && (millis() - lT) >= 10) {
                display.fillScreen(BG_COLOR);
                _drawScreen_(&display, 0, 0, DEBUG_SCREEN ? 0 : max(min(CURRENT_DEVICE+1, 2u), 1u));
                msOverlay(&display);
                SHOW_CHANGED = false;
                lT = millis();
            }

            //
            if ((millis() - oT) >= 1000) {
                msOverlay(&display);
                oT = millis();
            }
        }
    }

}

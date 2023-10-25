#pragma once

//
#ifdef ESP32
#include <thread>
//#include <SimplyAtomic.h>
#endif

//
#include <OLEDDisplay.h>
#include <OLEDDisplayFonts.h>
#include <OLEDDisplayUi.h>
#include <SH1106.h>
#include <SH1106Spi.h>
#include <SH1106Wire.h>
#include <SSD1306.h>
#include <SSD1306I2C.h>
#include <SSD1306Spi.h>
#include <SSD1306Wire.h>

//
#include "../time/rtc.hpp"
#include "../interface/current.hpp"
#include "../interface/i2c.hpp"

namespace oled {

    // Initialize the OLED display using Wire library
    static SSD1306Wire display(I2C_OLED_ADDR, I2C_SDA, I2C_SCL/*SDA, SCL*/);  // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
    static OLEDDisplayUi ui( &display );

    //
    void switchScreen(bool dbg, uint dvID) {
        if (DEBUG_SCREEN != dbg || CURRENT_DEVICE != dvID) {
            ui.switchToFrame(DEBUG_SCREEN ? 0 : max(min(CURRENT_DEVICE+1, 2u), 1u));
            DEBUG_SCREEN = dbg;
            CURRENT_DEVICE = std::max(std::min(dvID, 1u), 0u);
            ui.setFrameAnimation(/*SLIDE_LEFT*/SLIDE_LEFT);
            ui.setTimePerTransition(0);
            ui.transitionToFrame(dbg ? 0 : std::max(std::min(dvID+1, 2u), 1u));
            ui.setTimePerTransition(400);
        }
    }

    //
    #ifdef ESP32
    std::thread displayTask;
    void displayThread() {
        while(true) {
            //ATOMIC() {
                ui.update();
            //}
            delay(1);
        }
    }
    #endif


    //
    void _drawScreen_(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y, uint SCREEN_ID) {
        display->setFont(ArialMT_Plain_10);
        display->setTextAlignment(TEXT_ALIGN_LEFT);

        //
        display->drawString(0 + x, 11 + y, debug_info._LINE_[0].toString());
        display->drawString(0 + x, 22 + y, debug_info._LINE_[1].toString());
        display->drawString(0 + x, 33 + y, debug_info._LINE_[2].toString());
    }

    //
    void drawS0(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
        _drawScreen_(display, state, x, y, 0);
    }

    //
    void drawS1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
        _drawScreen_(display, state, x, y, 1);
    }

    //
    void drawS2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
        _drawScreen_(display, state, x, y, 2);
    }

    //
    void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
        display->setTextAlignment(TEXT_ALIGN_RIGHT);
        display->setFont(ArialMT_Plain_10);
        time_t _time_ = rtc::getTime();
        display->drawString(128, 0, String((_time_/3600)%24) + ":" + String((_time_/60)%60) + ":" + String(_time_%60));

        display->setTextAlignment(TEXT_ALIGN_LEFT);
        display->setFont(ArialMT_Plain_10);
        display->drawString(0, 0, DEBUG_SCREEN ? "Debug" : ("Device: " + String(CURRENT_DEVICE)));
    }

    //
    static FrameCallback frames[] = { drawS0, drawS1, drawS2 };
    static OverlayCallback overlays[] = { msOverlay };

    //
    void initDisplay() {
        //
        Serial.println("Init Display...");

        // The ESP is capable of rendering 60fps in 80Mhz mode
        // but that won't give you much time for anything else
        // run it in 160Mhz mode or just set it to 30 fps
        ui.setTargetFPS(60);
        //ui.setActiveSymbol(activeSymbol);
        //ui.setInactiveSymbol(inactiveSymbol);
        ui.setIndicatorPosition(BOTTOM);
        ui.setIndicatorDirection(LEFT_RIGHT);
        ui.setFrameAnimation(SLIDE_LEFT);
        ui.setFrames(frames, 3);
        ui.disableAutoTransition();
        ui.setOverlays(overlays, 1);
        ui.init();

        //
        display.flipScreenVertically();
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
    #ifndef ESP32
        ui.update();
    #endif

        //
    #ifdef ESP32
        Serial.println("Pinning to Core...");
        displayTask = std::thread(displayThread);
    #endif
    }

}
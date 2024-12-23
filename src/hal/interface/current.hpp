#pragma once

//
#include "../imports/imports.hpp"
#include "../hal/core/interface.hpp"

//
#include "../memory/f_string.hpp"

//
std::function<void(uint32_t)> COM_HANDLER;

//
static std::atomic<uint32_t> EXCEPTION;
static std::atomic<bool> LOADING_SD;
static std::atomic<uint> CURRENT_DEVICE;
static std::atomic<bool> DEBUG_SCREEN;
static std::atomic<uint32_t> BG_COLOR;
static const uintptr_t STOP_TIMEOUT = 1000;

//
static uintptr_t LAST_ACTIVE_TIME = millis();

//
std::atomic<bool> INTERRUPTED;
std::atomic<uintptr_t> LAST_TIME; //= millis();

//
std::atomic<bool> SHOW_CHANGED;
std::atomic<bool> POWER_SAVING;
std::atomic<bool> OVERLAY_CHANGED;

//
void wakeUp() {
    SHOW_CHANGED = false;
    digitalWrite(PIN_POWER_ON, HIGH);
    digitalWrite(PIN_LCD_BL, HIGH);
    setCpuFrequencyMhz(240);
    LAST_ACTIVE_TIME = millis();
    POWER_SAVING = false;
    SHOW_CHANGED = true;
}

//
void powerSave() {
    SHOW_CHANGED = false;
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);
    setCpuFrequencyMhz(80);
    POWER_SAVING = true;
}

//
void initState() {
    SHOW_CHANGED = false;
    EXCEPTION = -1;
    OVERLAY_CHANGED = false;
    POWER_SAVING = true;
    BG_COLOR = 0x0000;
    CURRENT_DEVICE = 0;
    DEBUG_SCREEN = true;
    LAST_TIME = millis();
    INTERRUPTED = false;
    LOADING_SD = false;
    SHOW_CHANGED = true;
    setCpuFrequencyMhz(80);
}

//
void _STOP_EXCEPTION_() {
    SHOW_CHANGED = false;
    LAST_TIME = millis();
    BG_COLOR = 0xF800;
    INTERRUPTED = true;
    SHOW_CHANGED = true;
}

//
struct DEBUG_INFO {
    _String_<> _LINE_[3];
} debug_info;

//
struct OVERLAY {
    _String_<> _LEFT_;
    _String_<> _RIGHT_;
} overlay;



//
void _LOG_(const uint8_t L, String const& string) {
    SHOW_CHANGED = false;
    debug_info._LINE_[L] = string;
    SHOW_CHANGED = true;
}

//
void switchScreen(bool dbg, uint dvID) {
    if (DEBUG_SCREEN != dbg || CURRENT_DEVICE != dvID) {
        SHOW_CHANGED = false;
        DEBUG_SCREEN = dbg;
        CURRENT_DEVICE = std::max(std::min(dvID, 1u), 0u);
        SHOW_CHANGED = true;
    }
}


#pragma once

//
#include "../memory/f_string.hpp"
#include "./pin_config.hpp"

//
#include <functional>
#include <utility>
#include <atomic>

//
std::function<void(uint32_t)> handler;

//
static std::atomic<bool> LOADING_SD;
static std::atomic<uint> CURRENT_DEVICE;
static std::atomic<bool> DEBUG_SCREEN;
static std::atomic<uint32_t> BG_COLOR;
static const uintptr_t STOP_TIMEOUT = 1000;

//
std::atomic<bool> INTERRUPTED;
std::atomic<uintptr_t> LAST_TIME; //= millis();

//
void initState() {
    BG_COLOR = 0x0000;
    CURRENT_DEVICE = 0;
    DEBUG_SCREEN = true;
    LAST_TIME = millis();
    INTERRUPTED = false;
    LOADING_SD = false;
}

//
void _STOP_EXCEPTION_() {
    LAST_TIME = millis();
    BG_COLOR = 0xF800;
    INTERRUPTED = true;
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
    debug_info._LINE_[L] = string;
    delay(10);
}

//
void switchScreen(bool dbg, uint dvID) {
    if (DEBUG_SCREEN != dbg || CURRENT_DEVICE != dvID) {
        DEBUG_SCREEN = dbg;
        CURRENT_DEVICE = std::max(std::min(dvID, 1u), 0u);
    }
}


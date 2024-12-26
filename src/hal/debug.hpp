#pragma once

//
#include <esp_log.h>
#include <std/string.hpp>
#include "./current.hpp"

//
struct DEBUG_INFO {
    _String_<> _LINE_[3];
} debug_info;

//
#ifdef ENABLE_ARDUINO
void _LOG_(const uint8_t L, String const& string) {
    SHOW_CHANGED = false;
    debug_info._LINE_[L] = string;
    SHOW_CHANGED = true;
}
#endif

//
void _STOP_EXCEPTION_() {
    SHOW_CHANGED = false;
#ifdef ENABLE_ARDUINO
    LAST_TIME = millis();
#endif
    BG_COLOR = 0xF800;
    INTERRUPTED = true;
    SHOW_CHANGED = true;
}

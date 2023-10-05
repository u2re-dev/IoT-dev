#pragma once

//
#include "../utils/f_string.hpp"

//
#include <functional>
#include <utility>
#include <atomic>

//
struct SCREEN {
    _String_<> _LINE_1_; //= SafeString(24, nullptr, nullptr);
    _String_<> _LINE_2_; //= SafeString(24, nullptr, nullptr);
    _String_<> _LINE_3_; //= SafeString(24, nullptr, nullptr);
};

//
static SCREEN _screen_[3];
static std::atomic<uint> CURRENT_DEVICE; //= 0;
static std::atomic<bool> DEBUG_SCREEN;// = true;

//
void initScreen() {
    CURRENT_DEVICE = 0;
    DEBUG_SCREEN = true;
}

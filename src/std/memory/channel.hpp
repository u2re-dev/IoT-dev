#pragma once

//
#include <library/std.hpp>

//
namespace channel {
    static const size_t LIMIT = 1024;
    thread_local char* _debug_ = 0;

    //
    thread_local size_t _r_length_ = 0;
    thread_local size_t _s_length_ = 0;

    //
    thread_local uint8_t* _received_ = 0;
    thread_local uint8_t* _sending_ = 0;
};

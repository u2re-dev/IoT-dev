#pragma once
#include <std/types.hpp>

//
namespace tc {

    //
    inline void binary_hex(uint8_t const* pin, char* pout, size_t blen) {
        const char * hex = "0123456789ABCDEF";
        for (size_t i = 0; i < blen; i++) {
            *pout++ = hex[(*pin>>4)&0xF];
            *pout++ = hex[(*pin++)&0xF];
        }
        *pout = 0;
    }

    //
    inline uint32_t bswap32(uint32_t const& num) {
        return ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);
    }

    //
    inline void store32(uint32_t* ptr, uint32_t _a_) {
        *(reinterpret_cast<uint8_t*>(ptr)+0) = (_a_>>0 )&0xFF;
        *(reinterpret_cast<uint8_t*>(ptr)+1) = (_a_>>8 )&0xFF;
        *(reinterpret_cast<uint8_t*>(ptr)+2) = (_a_>>16)&0xFF;
        *(reinterpret_cast<uint8_t*>(ptr)+3) = (_a_>>24)&0xFF;
    }

    //
    inline void store32(uint8_t* ptr, uint32_t _a_) {
        *(ptr+0) = (_a_>>0)&0xFF;
        *(ptr+1) = (_a_>>8)&0xFF;
        *(ptr+2) = (_a_>>16)&0xFF;
        *(ptr+3) = (_a_>>24)&0xFF;
    }

};

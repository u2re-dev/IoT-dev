#pragma once
#include <std/types.hpp>

//
namespace tc {

    //
    inline void binary_hex(uint8_t const* pin, char* pout, size_t blen) {
        const char * hex = "0123456789ABCDEF";
        int i = 0;
        for(; i < blen-1; ++i){
            *pout++ = hex[(*pin>>4)&0xF];
            *pout++ = hex[(*pin++)&0xF];
            //*pout++ = ':';
        }
        *pout++ = hex[(*pin>>4)&0xF];
        *pout++ = hex[(*pin)&0xF];
        *pout = 0;
    }

    //
    inline uint32_t bswap32(uint32_t const& num) {
        return ((num>>24)&0xff) | // move byte 3 to byte 0
               ((num<<8)&0xff0000) | // move byte 1 to byte 2
               ((num>>8)&0xff00) | // move byte 2 to byte 1
               ((num<<24)&0xff000000); // byte 0 to byte 3
    }

    //
    inline void store32(uint32_t* ptr, uint32_t _a_) {
        *((uint8_t*)(ptr)+0) = _a_&0xFF;
        *((uint8_t*)(ptr)+1) = (_a_>>8)&0xFF;
        *((uint8_t*)(ptr)+2) = (_a_>>16)&0xFF;
        *((uint8_t*)(ptr)+3) = (_a_>>24)&0xFF;
    }

    //
    inline void store32(uint8_t* ptr, uint32_t _a_) {
        *(ptr+0) = _a_&0xFF;
        *(ptr+1) = (_a_>>8)&0xFF;
        *(ptr+2) = (_a_>>16)&0xFF;
        *(ptr+3) = (_a_>>24)&0xFF;
    }

};

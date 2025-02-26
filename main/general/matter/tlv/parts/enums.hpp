#ifndef C6C745D5_626B_4A22_B876_9A1DCA736686
#define C6C745D5_626B_4A22_B876_9A1DCA736686

//
#include <cstdint>

//
namespace tlvcpp {
    //
    enum e_octet : uint8_t {
        BYTE  = 0b00,
        SHORT = 0b01,
        WORD  = 0b10, // I don't know how it name (32-bit values)...
        LONG  = 0b11
    };

    //
    enum e_float : uint8_t {
        FALSE = 0b00,
        TRUE  = 0b01,
        F32 = 0b10,
        F64 = 0b11
    };

    //
    enum e_struct : uint8_t {
        NULL_T = 0b00,
        STRUCT = 0b01,
        ARRAY  = 0b10,
        PATH   = 0b11
    };

    //
    enum e_type : uint8_t {
        SIGNED_INTEGER   = 0b000,
        UNSIGNED_INTEGER = 0b001,
        FLOATING_POINT   = 0b010,
        UTF8_STRING      = 0b011,
        BYTE_STRING      = 0b100,
        STRUCTURE        = 0b101, // itself is null
        END              = 0b110,
        UNKNOWN          = 0b111
    };
}

//
#endif

#ifndef C6C745D5_626B_4A22_B876_9A1DCA736686
#define C6C745D5_626B_4A22_B876_9A1DCA736686

//
#include <cstdint>

//
namespace tlvcpp {
    namespace e_type {
        enum Type : uint8_t {
            SIGNED_INTEGER        = 0x00,
            UNSIGNED_INTEGER      = 0x04,
            BOOLEAN               = 0x08,
            FLOATING_POINT_NUMBER = 0x0A,
            UTF8_STRING           = 0x0C,
            BYTE_STRING           = 0x10,
            STRUCTURE             = 0x14, // itself is null
            END                   = 0x18,
            //HAS                   = 0x15,
            //ARRAY                 = 0x16,
            //PATH                  = 0x17
        };
    }
}

#endif /* C6C745D5_626B_4A22_B876_9A1DCA736686 */

#ifndef D51DB55E_8F96_457F_B8D6_745D840571DD
#define D51DB55E_8F96_457F_B8D6_745D840571DD
#include <cstdint>

//
enum class SessionType : uint8_t {
    Unicast = 0,
    Group   = 1,
};

//
enum SecurityFlag : uint8_t {
    HasPrivacyEnhancements = 0b10000000,
    IsControlMessage       = 0b01000000,
    HasMessageExtension    = 0b00100000,
};

#endif /* D51DB55E_8F96_457F_B8D6_745D840571DD */

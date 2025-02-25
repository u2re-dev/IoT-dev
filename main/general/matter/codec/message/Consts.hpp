#ifndef D51DB55E_8F96_457F_B8D6_745D840571DD
#define D51DB55E_8F96_457F_B8D6_745D840571DD
#include <cstdint>
enum class SessionType : uint8_t { Unicast = 0, Group = 1 };
constexpr uint8_t  HEADER_VERSION = 0x00;
constexpr uint16_t COMMON_VENDOR_ID = 0x0000;
constexpr uint32_t SECURE_CHANNEL_PROTOCOL_ID = 0x0001;
constexpr uint32_t INTERACTION_PROTOCOL_ID    = 0x0002;
#endif

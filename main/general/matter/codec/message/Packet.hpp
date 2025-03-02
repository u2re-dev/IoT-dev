#ifndef D447913F_0CE7_41BB_80A2_1D22E109D71A
#define D447913F_0CE7_41BB_80A2_1D22E109D71A

//
#include <std/types.hpp>
#include <optional>
#include "./Consts.hpp"

//
struct exch_f {
    uint8_t hasDestNodeId: 1;
    uint8_t hasDestGroupId: 1;
    uint8_t hasSourceNodeId: 1;
    uint8_t reserved: 1;
    uint8_t version: 4;
};

//
struct secr_f {
    uint8_t sessionType: 2;
    uint8_t unknown: 3;
    uint8_t hasMessageExtensions: 1;
    uint8_t isControlMessage: 1;
    uint8_t hasPrivacyEnhancements: 1;
};

//
struct PacketHeader {
    uint32_t messageId      = 0;
    uint16_t sessionId      = 0;
    secr_f   securityFlags  = {0, 0, 0, 0, 0};
    exch_f   exchangeFlags  = {0, 0, 0, 0, HEADER_VERSION};

    //
    std::optional<uint16_t> destGroupId;
    std::optional<uint64_t> sourceNodeId;
    std::optional<uint64_t> destNodeId;
};

//
#endif

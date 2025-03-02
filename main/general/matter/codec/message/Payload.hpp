#ifndef C165ED21_5BB9_474A_8928_9AF5D7CACE24
#define C165ED21_5BB9_474A_8928_9AF5D7CACE24

//
#include <std/types.hpp>
#include <optional>

//
#include "../../tlv/tlv_tree.h"

//
#pragma pack(push, 1)
struct exch_f {
    uint8_t isInitiatorMessage: 1;
    uint8_t isAckMessage: 1;
    uint8_t requiresAck: 1;
    uint8_t hasSecureExtension: 1;
    uint8_t hasVendorId: 1;
};
#pragma pack(pop)

//
#pragma pack(push, 1)
struct PayloadHeader {
    exch_f   exchangeFlags  = {0, 0, 0, 0, 0};
    uint8_t  protocolCode   = 0;
    uint16_t exchangeId     = 0;
    uint16_t protocolId     = 0;
    uint32_t ackedMessageId = 0;
    uint16_t vendorId       = 0;
};
#pragma pack(pop)

//
#pragma pack(push, 1)
struct Payload {
    PayloadHeader header = {};
    bytespan_t securityExtension = {};
    bytespan_t payload = {};
    tlvcpp::tlv_tree_node TLV = {};
};
#pragma pack(pop)

//
#endif

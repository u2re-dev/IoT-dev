#ifndef C165ED21_5BB9_474A_8928_9AF5D7CACE24
#define C165ED21_5BB9_474A_8928_9AF5D7CACE24

//
#include <std/types.hpp>
#include <optional>

//
#include "../../tlv/tlv_tree.h"

//
struct msg_f {
    uint8_t isInitiatorMessage: 1;
    uint8_t isAckMessage: 1;
    uint8_t requiresAck: 1;
    uint8_t hasSecureExtension: 1;
    uint8_t hasVendorId: 1;
};

//
struct PayloadHeader {
    msg_f    messageFlags   = {0, 0, 0, 0, 0};
    uint16_t exchangeId     = 0;
    uint32_t protocolId     = 0;
    uint32_t ackedMessageId = 0;
    uint16_t vendorId       = 0;
     uint8_t messageType    = 0;
};

//
struct Payload {
    PayloadHeader header = {};
    bytespan_t securityExtension = {};
    bytespan_t payload = {};
    tlvcpp::tlv_tree_node TLV = {};
};

//
#endif

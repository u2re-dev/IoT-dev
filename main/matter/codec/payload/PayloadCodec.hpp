#pragma once

//
#include "../core/STD.hpp"

//
enum PayloadHeaderFlag : uint8_t {
    IsInitiatorMessage = 0b00000001,
    IsAckMessage       = 0b00000010,
    RequiresAck        = 0b00000100,
    HasSecureExtension = 0b00001000,
    HasVendorId        = 0b00010000,
};

//
struct PayloadHeader {
    uint16_t exchangeId;
    uint32_t protocolId;
    uint8_t messageType;
    bool isInitiatorMessage;
    bool requiresAck;
    std::optional<uint32_t> ackedMessageId;
    bool hasSecuredExtension;
};

//
struct PayloadCodec {
    static inline Packet encodePayload(const Message& message);
    static inline DecodedMessage decodePayload(const DecodedPacket& packet);
private:
    static inline PayloadHeader decodePayloadHeader(DataReader& reader);
    static inline ByteArray encodePayloadHeader(const PayloadHeader& ph);
};

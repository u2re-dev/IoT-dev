#pragma once

//
#include "../../../std/types.hpp"

//
enum PacketHeaderFlag : uint8_t {
    HasDestNodeId   = 0b00000001,
    HasDestGroupId  = 0b00000010,
    HasSourceNodeId = 0b00000100,
    Reserved        = 0b00001000,
    VersionMask     = 0b11110000,
};

//
struct PacketHeader {
    uint32_t messageId      = 0;
    uint16_t sessionId      = 0;
    uint16_t destGroupId    = 0;
    uint64_t sourceNodeId   = 0;
    uint64_t destNodeId     = 0;
    uint8_t  sessionType    = 0;
    uint8_t  securityFlags  = 0;

    //
    bool hasPrivacyEnhancements = false;
    bool isControlMessage       = false;
    bool hasMessageExtensions   = false;
};

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
    uint16_t exchangeId     = 0;
    uint32_t protocolId     = 0;
    uint32_t ackedMessageId = 0;
    uint16_t vendorId       = 0;
     uint8_t messageType    = 0;

    //
    bool isInitiatorMessage  = false;
    bool requiresAck         = false;
    bool hasSecuredExtension = false;
};



//
struct SecureMessageType {
    static inline std::string toString(uint8_t msgType) {
        return "SC_Message_" + std::to_string(msgType);
    }
};

//
struct MessageType {
    static inline std::string toString(uint8_t msgType) {
        return "I_Message_" + std::to_string(msgType);
    }
};

//
struct Payload {
    PayloadHeader header = {};
    bytes_t payload = {};
    bytes_t securityExtension = {};
};

//
struct Message {
    PacketHeader header = {};
    Payload decodedPayload = {};

    //
    bytes_t messageExtension = {};
    bytes_t rawPayload = {};
};

//
struct MsgTypeInfo {
    std::string type = "";
    std::string forType = "";
};

//
struct MessageCodec {
    static Message decodeMessage(reader_t& packet);
    static bytes_t encodeMessage(Message& message);
    static Message buildMessage(PacketHeader const& header, Payload const& payload);

    //
    static bytes_t encodePayload(Payload const& payload);
    static Payload decodePayload(reader_t& data);
private:
    static PacketHeader decodePacketHeader(reader_t& reader);
    static writer_t encodePacketHeader(const PacketHeader& ph);

    //
    static PayloadHeader decodePayloadHeader(reader_t& reader);
    static writer_t encodePayloadHeader(const PayloadHeader& ph);
};

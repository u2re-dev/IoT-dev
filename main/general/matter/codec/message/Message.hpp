#pragma once

//
#include <std/types.hpp>
#include <tlv/tlv_tree.h>
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
struct sec_f {
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
     sec_f   securityFlags  = {0, 0, 0, 0, 0};
    exch_f   exchangeFlags  = {0, 0, 0, 0, HEADER_VERSION};

    //
    std::optional<uint16_t> destGroupId;
    std::optional<uint64_t> sourceNodeId;
    std::optional<uint64_t> destNodeId;
};

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
    bytespan_t payload = {};
    bytespan_t securityExtension = {};
    tlvcpp::tlv_tree_node TLV = {};
};

//
struct Message {
    PacketHeader header = {};
    Payload decodedPayload = {};

    //
    bytespan_t messageExtension = {};
    bytespan_t rawPayload = {};
};

//
struct MsgTypeInfo {
    std::string type = "";
    std::string forType = "";
};

//
struct MessageCodec {
    static Message decodeMessage(reader_t& packet);
    static bytespan_t encodeMessage(Message& message);
    static Message buildMessage(PacketHeader const& header, Payload const& payload);

    //
    //static bytespan_t encodePayload(Payload const& payload); // const (variant I)
    static bytespan_t encodePayload(Payload& payload); // non-const (variant II)
    static Payload decodePayload(reader_t& data);
private:
    static PacketHeader decodePacketHeader(reader_t& reader);
    static writer_t encodePacketHeader(PacketHeader& ph);

    //
    static PayloadHeader decodePayloadHeader(reader_t& reader);
    static writer_t encodePayloadHeader(PayloadHeader& ph);
};

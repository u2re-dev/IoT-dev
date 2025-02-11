#pragma once

//
#include "../core/STD.hpp"
#include "../core/Consts.hpp"
#include "../core/Types.hpp"
#include "../packet/PacketCodec.hpp"
#include "../payload/PayloadCodec.hpp"

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
struct Message {
    PacketHeader packetHeader;
    PayloadHeader payloadHeader;
    ByteArray securityExtension;
    ByteArray payload;
};

//
struct DecodedMessage : public Message {
    DecodedPacketHeader packetHeader;
};

//
struct MsgTypeInfo {
    std::string type;
    std::string forType;
};

//
inline MsgTypeInfo mapProtocolAndMessageType(uint32_t protocolId, uint8_t messageType) {
    std::string msgTypeHex = Diagnostic::hex(messageType);
    std::string type = Diagnostic::hex(protocolId) + "/" + msgTypeHex;
    if (protocolId == SECURE_CHANNEL_PROTOCOL_ID)
        { return { type, "SC/" + SecureMessageType::toString(messageType) }; }
    else if (protocolId == INTERACTION_PROTOCOL_ID)
        { return { type, "I/" + MessageType::toString(messageType) }; }
    return { type, "" };
}

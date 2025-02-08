#pragma once

//
#include "../core/STD.hpp"
#include "../core/Types.hpp"
#include "../core/Utils.hpp"

//
enum PacketHeaderFlag : uint8_t {
    HasDestNodeId   = 0b00000001,
    HasDestGroupId  = 0b00000010,
    HasSourceNodeId = 0b00000100,
    Reserved        = 0b00001000,
    VersionMask     = 0b11110000,
};


// Структуры, соответствующие интерфейсам
struct PacketHeader {
    uint16_t sessionId;
    SessionType sessionType;
    bool hasPrivacyEnhancements;
    bool isControlMessage;
    bool hasMessageExtensions;
    uint32_t messageId;
    std::optional<NodeId> sourceNodeId;
    std::optional<NodeId> destNodeId;
    std::optional<GroupId> destGroupId;
};

//
struct Packet {
    PacketHeader header;
    std::optional<ByteArray> messageExtension;
    ByteArray applicationPayload;
};

//
struct DecodedPacketHeader : public PacketHeader {
    uint8_t securityFlags; // используется как nonce
};

//
struct DecodedPacket : public Packet {
    DecodedPacketHeader header;
};


//
struct PacketCodec {
    static inline DecodedPacket decodePacket(const ByteArray& data);
    static inline ByteArray encodePacket(const Packet& packet) ;
private:
    static inline DecodedPacketHeader decodePacketHeader(DataReader& reader);
    static inline ByteArray encodePacketHeader(const PacketHeader& ph);
};

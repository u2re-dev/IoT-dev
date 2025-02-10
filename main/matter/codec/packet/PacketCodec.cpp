#include "./PacketCodec.hpp"
#include "../core/Utils.hpp"
#include "../core/Types.hpp"

//
inline DecodedPacketHeader PacketCodec::decodePacketHeader(DataReader& reader) {
    DecodedPacketHeader header{};
    uint8_t flags   = reader.readUInt8();
    uint8_t version = (flags & VersionMask) >> 4;

    //
    bool hasDN = (flags & HasDestNodeId)   != 0;
    bool hasDG = (flags & HasDestGroupId)  != 0;
    bool hasSN = (flags & HasSourceNodeId) != 0;

    //
    if (hasDN && hasDG)            throw UnexpectedDataError("Header cannot contain both destination node and group.");
    if (version != HEADER_VERSION) throw NotImplementedError("Unsupported header version " + std::to_string(version) + ".");

    //
    header.sessionId     = reader.readUInt16();
    header.securityFlags = reader.readUInt8();
    header.messageId     = reader.readUInt32();

    //
    if (hasSN) header.sourceNodeId = reader.readUInt64();
    if (hasDN) header.destNodeId   = reader.readUInt64();
    if (hasDG) header.destGroupId  = static_cast<GroupId>(reader.readUInt16());

    //
    uint8_t sessionTypeVal = header.securityFlags & 0b00000011;
    if (sessionTypeVal != static_cast<uint8_t>(SessionType::Group) &&
        sessionTypeVal != static_cast<uint8_t>(SessionType::Unicast))
        throw UnexpectedDataError("Unsupported session type " + std::to_string(sessionTypeVal));

    //
    header.sessionType              = static_cast<SessionType>(sessionTypeVal);
    header.hasPrivacyEnhancements   = (header.securityFlags & HasPrivacyEnhancements) != 0;
    header.isControlMessage         = (header.securityFlags & IsControlMessage)       != 0;
    header.hasMessageExtensions     = (header.securityFlags & HasMessageExtension)    != 0;

    //
    if (header.hasPrivacyEnhancements)  throw NotImplementedError("Privacy enhancements not supported");
    if (header.isControlMessage)        throw NotImplementedError("Control Messages not supported");

    //
    return header;
}

//
inline ByteArray PacketCodec::encodePacketHeader(const PacketHeader& ph) {
    DataWriter writer;

    //
    uint8_t flags = (HEADER_VERSION << 4) |
                    (ph.destGroupId  ? HasDestGroupId  : 0) |
                    (ph.destNodeId   ? HasDestNodeId   : 0) |
                    (ph.sourceNodeId ? HasSourceNodeId : 0);

    //
    writer.writeUInt8(flags);
    writer.writeUInt16(ph.sessionId);
    writer.writeUInt8(static_cast<uint8_t>(ph.sessionType));
    writer.writeUInt32(ph.messageId);

    //
    if (ph.sourceNodeId) writer.writeUInt64(ph.sourceNodeId);
    if (ph.destNodeId)   writer.writeUInt64(ph.destNodeId);
    if (ph.destGroupId)  writer.writeUInt16(ph.destGroupId);

    //
    return writer.toByteArray();
}



//
inline DecodedPacket PacketCodec::decodePacket(const ByteArray& data) {
    DataReader reader(data);
    DecodedPacketHeader header = decodePacketHeader(reader);

    //
    DecodedPacket dp;
    dp.header             = header;
    dp.messageExtension   = header.hasMessageExtensions ? reader.readByteArray(reader.readUInt16()) : ByteArray{};
    dp.applicationPayload = reader.remainingBytes();
    return dp;
}

//
inline ByteArray PacketCodec::encodePacket(const Packet& packet) {
    if (packet.messageExtension.size() || packet.header.hasMessageExtensions) 
        throw NotImplementedError("Message extensions not supported when encoding a packet.");
    return Bytes::concat({encodePacketHeader(packet.header), packet.applicationPayload});
}

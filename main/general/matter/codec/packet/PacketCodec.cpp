#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"

//
PacketHeader MessageCodec::decodePacketHeader(reader_t& reader) {
    PacketHeader header{};

    //
    header.exchangeFlags = reinterpret_cast<exch_f const&>(reader.readUInt8());
    if (header.exchangeFlags.hasDestNodeId && header.exchangeFlags.hasDestGroupId) throw UnexpectedDataError("Header cannot contain both destination node and group.");
    if (header.exchangeFlags.version != HEADER_VERSION) throw NotImplementedError("Unsupported header version " + std::to_string(header.exchangeFlags.version) + ".");

    //
    header.sessionId     = reader.readUInt16();
    header.securityFlags = reinterpret_cast<sec_f const&>(reader.readUInt8());
    header.messageId     = reader.readUInt32();

    //
    if (header.exchangeFlags.hasSourceNodeId) header.sourceNodeId = reader.readUInt64();
    if (header.exchangeFlags.hasDestNodeId)   header.destNodeId   = reader.readUInt64();
    if (header.exchangeFlags.hasDestGroupId)  header.destGroupId  = static_cast<uint16_t>(reader.readUInt16());

    //
    if (header.securityFlags.sessionType != static_cast<uint8_t>(SessionType::Group) && header.securityFlags.sessionType != static_cast<uint8_t>(SessionType::Unicast))
        throw UnexpectedDataError("Unsupported session type " + std::to_string(header.securityFlags.sessionType));

    //
    if (header.securityFlags.hasPrivacyEnhancements)  throw NotImplementedError("Privacy enhancements not supported");
    if (header.securityFlags.isControlMessage)        throw NotImplementedError("Control Messages not supported");

    //
    return header;
}

//
writer_t MessageCodec::encodePacketHeader(PacketHeader& ph) {
    writer_t writer;

    // TODO: use different principle
    if (ph.sourceNodeId.has_value()) ph.exchangeFlags.hasSourceNodeId = 1;
    if (ph.destNodeId.has_value())   ph.exchangeFlags.hasDestNodeId = 1;
    if (ph.destGroupId.has_value())  ph.exchangeFlags.hasDestGroupId = 1;

    //
    writer.writeUInt8(reinterpret_cast<uint8_t const&>(ph.exchangeFlags));
    writer.writeUInt16(ph.sessionId);
    writer.writeUInt8(reinterpret_cast<uint8_t const&>(ph.securityFlags));
    writer.writeUInt32(ph.messageId);

    // TODO: use different principle
    if (ph.exchangeFlags.hasSourceNodeId) writer.writeUInt64(ph.sourceNodeId.value_or(0));
    if (ph.exchangeFlags.hasDestNodeId)   writer.writeUInt64(ph.destNodeId.value_or(0));
    if (ph.exchangeFlags.hasDestGroupId)  writer.writeUInt16(ph.destGroupId.value_or(0));

    //
    return writer;
}



//
Message MessageCodec::decodeMessage(reader_t& reader) {
    Message dp;
    dp.header             = decodePacketHeader(reader);;
    dp.messageExtension   = dp.header.securityFlags.hasMessageExtensions ? bytespan_t(reader.readBytes(reader.readUInt16())) : bytespan_t{};
    dp.rawPayload         = reader.remainingBytes();
    return dp;
}

//
bytespan_t MessageCodec::encodeMessage(Message& packet) {
    if (packet.messageExtension && (packet.messageExtension->size() || packet.header.securityFlags.hasMessageExtensions)) throw NotImplementedError("Message extensions not supported when encoding a packet.");
    if (!packet.rawPayload) packet.rawPayload = encodePayload(packet.decodedPayload);
    return concat({encodePacketHeader(packet.header), packet.rawPayload});
}

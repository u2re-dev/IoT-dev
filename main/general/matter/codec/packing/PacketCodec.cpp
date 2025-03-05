#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"

//
PacketHeader MessageCodec::decodePacketHeader(reader_t& reader) {
    PacketHeader header = {};

    //
    header.messageFlags = reinterpret_cast<msg_f const&>(reader.readUInt8());
    if (header.messageFlags.hasDestNodeId && header.messageFlags.hasDestGroupId) throw UnexpectedDataError("Header cannot contain both destination node and group.");
    if (header.messageFlags.version != HEADER_VERSION) throw NotImplementedError("Unsupported header version " + std::to_string(header.messageFlags.version) + ".");

    //
    header.sessionId     = reader.readUInt16();
    header.securityFlags = reinterpret_cast<sec_f const&>(reader.readUInt8());
    header.messageId     = reader.readUInt32();

    //
    if (header.messageFlags.hasSourceNodeId) header.sourceNodeId = reader.readUInt64();
    if (header.messageFlags.hasDestNodeId)   header.destNodeId   = reader.readUInt64();
    if (header.messageFlags.hasDestGroupId)  header.destGroupId  = static_cast<uint16_t>(reader.readUInt16());

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
    writer_t writer = {};

    // TODO: use different principle
    if (ph.sourceNodeId.has_value()) ph.messageFlags.hasSourceNodeId = 1;
    if (ph.destNodeId.has_value())   ph.messageFlags.hasDestNodeId = 1;
    if (ph.destGroupId.has_value())  ph.messageFlags.hasDestGroupId = 1;

    //
    writer.writeUInt8(reinterpret_cast<uint8_t const&>(ph.messageFlags));
    writer.writeUInt16(ph.sessionId);
    writer.writeUInt8(reinterpret_cast<uint8_t const&>(ph.securityFlags));
    writer.writeUInt32(ph.messageId);

    // TODO: use different principle
    if (ph.messageFlags.hasSourceNodeId) writer.writeUInt64(ph.sourceNodeId.value_or(0));
    if (ph.messageFlags.hasDestNodeId)   writer.writeUInt64(ph.destNodeId.value_or(0));
    if (ph.messageFlags.hasDestGroupId)  writer.writeUInt16(ph.destGroupId.value_or(0));

    //
    return writer;
}

//
Message MessageCodec::decodeMessage(reader_t& reader, SessionKeys const& keys) {
    Message dp = {};
    dp.header           = decodePacketHeader(reader);
    dp.messageExtension = dp.header.securityFlags.hasMessageExtensions ? bytespan_t(reader.readBytes(reader.readUInt16())) : bytespan_t{};
    dp.cryptPayload     = reader.remainingBytes(); bytespan_t aadWith = reader.getMemory();
    dp.decodedPayload   = MessageCodec::decodePayloadF(MessageCodec::decryptPayload(dp, aadWith, keys));
    return dp;
}

//
bytespan_t MessageCodec::encodeMessage(Message& pt, SessionKeys const& keys) {
    if (pt.messageExtension && (pt.messageExtension->size() || pt.header.securityFlags.hasMessageExtensions))
        { throw NotImplementedError("Message extensions not supported when encoding a packet."); };

    //
    auto aad = encodePacketHeader(pt.header);
    if (!pt.rawPayload) { pt.rawPayload = encodePayload(pt.decodedPayload); };
    if ( pt.rawPayload && !pt.cryptPayload)
        { pt.cryptPayload = encryptPayload(pt, aad, keys); }
    return concat({aad, pt.cryptPayload});
}

//
Message MessageCodec::buildMessage(PacketHeader const& header, Payload const& payload) {
    if (payload.header.exchangeFlags.hasSecureExtension)
        { throw NotImplementedError("Security extensions not supported when encoding a payload."); }

    //
    Message pkt = {};
    pkt.header         = header;
    pkt.decodedPayload = payload;
    return pkt;
}

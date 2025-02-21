#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"

//#include "../core/Utils.hpp"
//#include "../core/Types.hpp"

//
writer_t MessageCodec::encodePayloadHeader(const PayloadHeader& ph) {
    writer_t writer;
    uint16_t vendorId = static_cast<uint16_t>((ph.protocolId >> 16) & 0xffff);
    uint8_t flags = (ph.isInitiatorMessage ? IsInitiatorMessage : 0) |
                    (ph.ackedMessageId     ? IsAckMessage       : 0) |
                    (ph.requiresAck        ? RequiresAck        : 0) |
                    (vendorId != COMMON_VENDOR_ID ? HasVendorId : 0);
    writer.writeUInt8(flags);
    writer.writeUInt8(ph.messageType);
    writer.writeUInt16(ph.exchangeId);
    if (vendorId != COMMON_VENDOR_ID) { writer.writeUInt32(ph.protocolId); } else { writer.writeUInt16(static_cast<uint16_t>(ph.protocolId)); };
    if (ph.ackedMessageId) writer.writeUInt32(ph.ackedMessageId);
    return writer;
}

//
PayloadHeader MessageCodec::decodePayloadHeader(reader_t& reader) {
    PayloadHeader ph{};
    uint8_t exchFlags = reader.readByte();
    bool isAck     = (exchFlags & IsAckMessage) != 0;
    bool hasVendor = (exchFlags & HasVendorId ) != 0;

    //
    ph.isInitiatorMessage  = (exchFlags & IsInitiatorMessage) != 0;
    ph.hasSecuredExtension = (exchFlags & HasSecureExtension) != 0;
    ph.requiresAck         = (exchFlags & RequiresAck)  != 0;;
    ph.messageType = reader.readUInt8();
    ph.exchangeId  = reader.readUInt16();
    ph.vendorId    = hasVendor ? reader.readUInt16() : COMMON_VENDOR_ID;
    ph.protocolId  = reader.readUInt16(); //vendorId != COMMON_VENDOR_ID ? reader.readUInt32() : reader.readUInt16();

    //
    if (isAck) ph.ackedMessageId = reader.readUInt32();
    return ph;
}



// 
Payload MessageCodec::decodePayload(reader_t& reader) {
    Payload msg {};
    msg.header = decodePayloadHeader(reader);
    msg.securityExtension = msg.header.hasSecuredExtension ? bytespan_t(reader.readBytes(bswap16(reader.readUInt16()))) : bytespan_t{};
    msg.payload = reader.remainingBytes();
    return msg;
}

//
bytespan_t MessageCodec::encodePayload(Payload const& payload) {
    writer_t encodedPH = encodePayloadHeader(payload.header);
    return payload.payload ? concat({encodedPH, payload.payload}) : encodedPH.toBytes();
}

//
Message MessageCodec::buildMessage(PacketHeader const& header, Payload const& payload) {
    if (payload.header.hasSecuredExtension) throw NotImplementedError("Security extensions not supported when encoding a payload.");

    //
    Message pkt;
    pkt.header  = header;
    pkt.decodedPayload = payload; //encodePayload(payload);
    return pkt;
}

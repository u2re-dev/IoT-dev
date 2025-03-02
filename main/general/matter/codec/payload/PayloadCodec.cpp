#include <std/hex.hpp>

//
#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"



//
writer_t MessageCodec::encodePayloadHeader(PayloadHeader& ph) {
    writer_t writer = {};
    ph.messageFlags.isAckMessage = ph.ackedMessageId ? 1 : 0;
    ph.messageFlags.hasVendorId  = ph.vendorId != COMMON_VENDOR_ID ? 1 : 0;
    writer.writeUInt8(reinterpret_cast<uint8_t const&>(ph.messageFlags));
    writer.writeUInt8(ph.messageType);
    writer.writeUInt16(ph.exchangeId);
    if (ph.messageFlags.hasVendorId) writer.writeUInt16(ph.vendorId);
    writer.writeUInt16(static_cast<uint16_t>(ph.protocolId));
    if (ph.messageFlags.isAckMessage) writer.writeUInt32(ph.ackedMessageId);
    return writer;
}

//
PayloadHeader MessageCodec::decodePayloadHeader(reader_t& reader) {
    PayloadHeader ph = {};
    ph.messageFlags   = reinterpret_cast<msg_f const&>(reader.readByte());
    ph.messageType    = reader.readUInt8();
    ph.exchangeId     = reader.readUInt16();
    ph.vendorId       = ph.messageFlags.hasVendorId ? reader.readUInt16() : COMMON_VENDOR_ID;
    ph.protocolId     = reader.readUInt16();
    ph.ackedMessageId = ph.messageFlags.isAckMessage ? reader.readUInt32() : 0;
    return ph;
}



//
Payload MessageCodec::decodePayload(reader_t& reader) {
    Payload msg = {};
    msg.header = decodePayloadHeader(reader);
    msg.securityExtension = msg.header.messageFlags.hasSecureExtension ? bytespan_t(reader.readBytes(reader.readUInt16())) : bytespan_t{};
    msg.payload = reader.remainingBytes();
    if (msg.payload && msg.payload->size() > 0) {
        msg.TLV.deserialize(msg.payload);
    }
    return msg;
}

// variant II - write TLV as payload (when not const)
bytespan_t MessageCodec::encodePayload(Payload& payload) {
    writer_t encodedPH = encodePayloadHeader(payload.header);
    if (!payload.payload) { writer_t tlv = encodedPH; payload.TLV.serialize(tlv); payload.payload = tlv.toBytes(); return encodedPH.toBytes(); }
    return concat({encodedPH, payload.payload});
}

#include <std/hex.hpp>

//
#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "../diagnostic/Diagnostic.hpp"



//
writer_t MessageCodec::encodePayloadHeader(PayloadHeader& ph) {
    writer_t writer = {};
    ph.exchangeFlags.isAckMessage = ph.ackedMessageId ? 1 : 0;
    ph.exchangeFlags.hasVendorId  = ph.vendorId != COMMON_VENDOR_ID ? 1 : 0;
    writer.writeUInt8(reinterpret_cast<uint8_t const&>(ph.exchangeFlags));
    writer.writeUInt8(ph.protocolOpCode);
    writer.writeUInt16(ph.exchangeId);
    if (ph.exchangeFlags.hasVendorId) writer.writeUInt16(ph.vendorId);
    writer.writeUInt16(static_cast<uint16_t>(ph.protocolId));
    if (ph.exchangeFlags.isAckMessage) writer.writeUInt32(ph.ackedMessageId);
    return writer;
}

//
PayloadHeader MessageCodec::decodePayloadHeader(reader_t& reader) {
    PayloadHeader ph = {};
    ph.exchangeFlags  = reinterpret_cast<exch_f const&>(reader.readByte());
    ph.protocolOpCode = reader.readUInt8();
    ph.exchangeId     = reader.readUInt16();
    ph.vendorId       = ph.exchangeFlags.hasVendorId ? reader.readUInt16() : COMMON_VENDOR_ID;
    ph.protocolId     = reader.readUInt16();
    ph.ackedMessageId = ph.exchangeFlags.isAckMessage ? reader.readUInt32() : 0;
    return ph;
}



//
Payload MessageCodec::decodePayload(reader_t& reader) {
    Payload msg = {};
    msg.header = decodePayloadHeader(reader);
    msg.securityExtension = msg.header.exchangeFlags.hasSecureExtension ? bytespan_t(reader.readBytes(reader.readUInt16())) : bytespan_t{};
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

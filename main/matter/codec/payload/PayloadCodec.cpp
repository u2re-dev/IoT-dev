#include "./PayloadCodec.hpp"
#include "../core/Utils.hpp"
#include "../core/Types.hpp"

//
inline ByteArray PayloadCodec::encodePayloadHeader(const PayloadHeader& ph) {
    DataWriter writer;
    uint16_t vendorId = static_cast<uint16_t>((ph.protocolId >> 16) & 0xffff);
    uint8_t flags = (ph.isInitiatorMessage ? IsInitiatorMessage : 0) |
                    (ph.ackedMessageId.has_value() ? IsAckMessage : 0) |
                    (ph.requiresAck ? RequiresAck : 0) |
                    (vendorId != COMMON_VENDOR_ID ? HasVendorId : 0);
    writer.writeUInt8(flags);
    writer.writeUInt8(ph.messageType);
    writer.writeUInt16(ph.exchangeId);
    if (vendorId != COMMON_VENDOR_ID) writer.writeUInt32(ph.protocolId);
    else writer.writeUInt16(static_cast<uint16_t>(ph.protocolId));
    if (ph.ackedMessageId) writer.writeUInt32(*ph.ackedMessageId);
    return writer.toByteArray();
}

//
inline PayloadHeader PayloadCodec::decodePayloadHeader(DataReader& reader) {
    PayloadHeader ph{};
    uint8_t exchFlags = reader.readUInt8();
    bool isAck = (exchFlags & IsAckMessage) != 0;
    bool hasVendor = (exchFlags & HasVendorId) != 0;

    //
    ph.isInitiatorMessage = (exchFlags & IsInitiatorMessage) != 0;
    ph.requiresAck = (exchFlags & RequiresAck) != 0;
    ph.hasSecuredExtension = (exchFlags & HasSecureExtension) != 0;
    ph.messageType = reader.readUInt8();
    ph.exchangeId = reader.readUInt16();

    //
    uint16_t vendorId = hasVendor ? reader.readUInt16() : COMMON_VENDOR_ID;
    ph.protocolId = vendorId != COMMON_VENDOR_ID ? reader.readUInt32() : reader.readUInt16();

    //
    if (isAck) ph.ackedMessageId = reader.readUInt32();
    return ph;
}



// Декодирование полезной нагрузки
inline DecodedMessage PayloadCodec::decodePayload(const DecodedPacket& packet) {
    DataReader reader(packet.applicationPayload);
    PayloadHeader payloadHeader = decodePayloadHeader(reader);
    std::optional<ByteArray> secExt = std::nullopt;
    if (payloadHeader.hasSecuredExtension) secExt = reader.readByteArray(reader.readUInt16());
    
    //
    DecodedMessage msg;
    msg.packetHeader = packet.header;
    msg.payloadHeader = payloadHeader;
    msg.securityExtension = secExt;
    msg.payload = reader.remainingBytes();
    return msg;
}

// Кодирование полезной нагрузки
inline Packet PayloadCodec::encodePayload(const Message& message) {
    if (message.securityExtension.has_value() || message.payloadHeader.hasSecuredExtension) 
        throw NotImplementedError("Security extensions not supported when encoding a payload.");
    ByteArray encodedPH = encodePayloadHeader(message.payloadHeader);

    //
    Packet pkt;
    pkt.header = message.packetHeader;
    pkt.applicationPayload = Bytes::concat({encodedPH, message.payload});
    return pkt;
}

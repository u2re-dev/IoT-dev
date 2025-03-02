#pragma once

//
#include <std/types.hpp>
#include <optional>

//
#include "./Payload.hpp"
#include "./Packet.hpp"

//
struct Message {
    PacketHeader header = {};
    Payload decodedPayload = {};

    //
    bytespan_t messageExtension = {};
    bytespan_t rawPayload = {};
};

//
struct MessageCodec {
    static bytespan_t encodeMessage(Message& message);
    static Message buildMessage(PacketHeader const& header, Payload const& payload);
    static Message makeMessage(Message const& request, uint8_t messageType, bytespan_t const& set);
    static Message decodeMessageF(reader_t packet) { return decodeMessage(packet); };
    static Message decodeMessage(reader_t& packet);

    //
    static bytespan_t encodePayload(Payload& payload);
    static Payload decodePayloadF(reader_t data) { return decodePayload(data); };
    static Payload decodePayload(reader_t& data);

    //
    static Payload const& debugPayload(Payload const& message);
    static Message const& debugMessage(Message const& message);

    //
private:
    static PacketHeader decodePacketHeaderF(reader_t reader) { return decodePacketHeader(reader); };
    static PacketHeader decodePacketHeader(reader_t& reader);
    static PayloadHeader decodePayloadHeaderF(reader_t reader) { return decodePayloadHeader(reader); };
    static PayloadHeader decodePayloadHeader(reader_t& reader);
    static writer_t encodePacketHeader(PacketHeader& ph);
    static writer_t encodePayloadHeader(PayloadHeader& ph);
};

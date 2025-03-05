#pragma once

//
#include <std/types.hpp>
#include <optional>

//
#include "./Payload.hpp"
#include "./Packet.hpp"

//
#pragma pack(push, 1)
struct Message {
    PacketHeader header = {};
    Payload decodedPayload = {};

    //
    bytespan_t messageExtension = {};
    bytespan_t cryptPayload = {};
    bytespan_t rawPayload = {};
};
#pragma pack(pop)

//
struct SessionKeys  { intx::uint128 I2Rkeys = 0, R2Ikeys = 0, AttestationChallenge = 0; };
struct MessageCodec {
    static bytespan_t encodeMessage(Message& message, SessionKeys const& keys = {});
    static bytespan_t encodePayload(Payload& payload);

    //
    static Message buildMessage(PacketHeader const& header, Payload const& payload);
    static Message makeMessage(Message const& request, uint8_t messageType, bytespan_t const& set);
    static Message decodeMessageF(reader_t packet, SessionKeys const& keys = {}) { return decodeMessage(packet, keys); };
    static Message decodeMessage(reader_t& packet, SessionKeys const& keys = {});

    //
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

    //
    static bytespan_t& encryptPayload(Message& message, bytespan_t const& aad, SessionKeys const& keys = {});
    static bytespan_t& decryptPayload(Message& message, bytespan_t const& aad, SessionKeys const& keys = {});
};

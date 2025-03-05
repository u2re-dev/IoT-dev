#ifndef BA234649_B621_49F7_A8E9_F250EA2F4A9B
#define BA234649_B621_49F7_A8E9_F250EA2F4A9B

//
#include "./session.hpp"
#include "../../tlv/parts/debug.hpp"

// ProtocolCode::AckMessage (currently in PASE)
bytespan_t Session::makeAckMessage(Message const& request) {
    Message outMsg = makeMessage(request, 0x10);
    return MessageCodec::encodeMessage(outMsg, sessionKeys);
}

//
Message Session::makeMessage(Message const& request, uint8_t protocolCode, bytespan_t const& set) {
    Message outMsg = {};
    outMsg.header.messageId     = (counter++);
    outMsg.header.sessionId     = sessionId;
    outMsg.header.securityFlags = request.header.securityFlags;
    if (request.header.sourceNodeId) { outMsg.header.destNodeId = request.header.sourceNodeId; };
    if (request.header.destNodeId) { outMsg.header.sourceNodeId = request.header.destNodeId; };
    outMsg.decodedPayload.header.exchangeFlags.requiresAck = 0;
    outMsg.decodedPayload.header.vendorId     = request.decodedPayload.header.vendorId;
    outMsg.decodedPayload.header.exchangeId   = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId   = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.protocolCode = protocolCode;
    outMsg.decodedPayload.payload = set;
    if (request.decodedPayload.header.exchangeFlags.requiresAck) {
        outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    }
    return outMsg;
}

//
Message Session::makeMessage(Message const& request, uint8_t protocolCode, tlvcpp::tlv_tree_node const& TLV) {
    Message outMsg = {};
    outMsg.header.messageId     = (counter++);
    outMsg.header.sessionId     = sessionId;
    outMsg.header.securityFlags = request.header.securityFlags;
    if (request.header.sourceNodeId) { outMsg.header.destNodeId = request.header.sourceNodeId; };
    if (request.header.destNodeId) { outMsg.header.sourceNodeId = request.header.destNodeId; };
    outMsg.decodedPayload.header.exchangeFlags.requiresAck = 0;
    outMsg.decodedPayload.header.vendorId     = request.decodedPayload.header.vendorId;
    outMsg.decodedPayload.header.exchangeId   = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId   = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.protocolCode = protocolCode;
    outMsg.decodedPayload.TLV = TLV;
    if (request.decodedPayload.header.exchangeFlags.requiresAck) {
        outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    }
    return outMsg;
}

//
Message Session::decodeMessage(bytespan_t const& bytes) const {
    auto message = MessageCodec::decodeMessageF(bytes, sessionKeys);
    MessageCodec::debugMessage(message);
    MessageCodec::debugPayload(message.decodedPayload);
    tlvcpp::debug_print_recursive(message.decodedPayload.TLV);
    return message;
}

//
bytespan_t Session::encodeMessage(Message& message) const {
    return MessageCodec::encodeMessage(message, sessionKeys);
}

//
#endif

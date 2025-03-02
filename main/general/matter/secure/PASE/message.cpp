#ifndef A3E1DE45_C06A_44C2_9B2F_34F686879D28
#define A3E1DE45_C06A_44C2_9B2F_34F686879D28

//
#include "../PASE.hpp"

//
#include "../../tlv/parts/debug.hpp"



//
uint8_t PASE::handlePayload(Payload const& payload) {
    switch (payload.header.protocolOpCode) {
        case 0x20: return handlePASERequest(payload);
        case 0x22: return handlePAKE1(payload);
        case 0x24: return handlePAKE3(payload);
        default: return 0;
    }
    return payload.header.protocolOpCode;
}

//
Message PASE::decodeMessage(bytespan_t const& bytes) const {
    auto reader  = reader_t(bytes);
    auto message = MessageCodec::decodeMessage(reader);
    message.decodedPayload = MessageCodec::decodePayloadF(decryptPayload(message, bytes));
    MessageCodec::debugMessage(message);
    MessageCodec::debugPayload(message.decodedPayload);
    tlvcpp::debug_print_recursive(message.decodedPayload.TLV);
    return message;
}



//
Message PASE::makeMessage(Message const& request, uint8_t messageType, bytespan_t const& set) {
    Message outMsg = {};
    outMsg.header.messageId  = (counter++); ///- request.header.messageId;
    outMsg.header.sessionId  = request.header.sessionId;
    outMsg.header.destNodeId = request.header.sourceNodeId;
    outMsg.decodedPayload.header.protocolOpCode = messageType;
    outMsg.decodedPayload.header.exchangeFlags.requiresAck = 0;
    outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    outMsg.decodedPayload.payload = set; // planned to replace by TLV encoding
    return outMsg;
}

//
Message PASE::makeMessage(Message const& request, uint8_t messageType, tlvcpp::tlv_tree_node const& TLV) {
    Message outMsg = {};
    outMsg.header.messageId  = (counter++); ///- request.header.messageId;
    outMsg.header.sessionId  = request.header.sessionId;
    outMsg.header.destNodeId = request.header.sourceNodeId;
    outMsg.decodedPayload.header.protocolOpCode = messageType;
    outMsg.decodedPayload.header.exchangeFlags.requiresAck = 0;
    outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    outMsg.decodedPayload.TLV = TLV; // planned to replace by TLV encoding
    return outMsg;
}

//
#endif

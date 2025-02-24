#ifndef A3E1DE45_C06A_44C2_9B2F_34F686879D28
#define A3E1DE45_C06A_44C2_9B2F_34F686879D28

//
#include "../PASE.hpp"

//
uint8_t PASE::handlePayload(Payload const& payload) {
    switch (payload.header.messageType) {
        case 0x20: return handlePASERequest(payload);
        case 0x22: return handlePAKE1(payload);
        case 0x24: return handlePAKE3(payload);
        default: return 0;
    }
    return payload.header.messageType;
}

//
Message PASE::decodeMessage(bytespan_t const& bytes) const {
    auto reader = reader_t(bytes);
    auto message = MessageCodec::decodeMessage(reader);
    decryptPayload(message, bytes);
    message.decodedPayload = MessageCodec::decodePayload(reader);
    return message;
}

//
Message PASE::makeMessage(Message const& request, uint8_t messageType, bytespan_t const& payload) {
    Message outMsg = {};
    outMsg.header.messageId  = (counter++); ///- request.header.messageId;
    outMsg.header.sessionId  = request.header.sessionId;
    outMsg.header.destNodeId = request.header.sourceNodeId;
    outMsg.decodedPayload.header.messageType = messageType;
    outMsg.decodedPayload.header.requiresAck = false;//true;
    outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    outMsg.decodedPayload.payload = payload;
    return outMsg;
}

#endif /* A3E1DE45_C06A_44C2_9B2F_34F686879D28 */

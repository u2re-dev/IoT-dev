#ifndef BA234649_B621_49F7_A8E9_F250EA2F4A9B
#define BA234649_B621_49F7_A8E9_F250EA2F4A9B

//
#include "../channel.hpp"
#include "../../tlv/parts/debug.hpp"


//
bytespan_t Channel::makeAckMessage(Message const& request) {
    Message outMsg = makeMessage(request, 0x10);
    return MessageCodec::encodeMessage(outMsg);
}




//
Message Channel::makeMessage(Message const& request, uint8_t messageType, bytespan_t const& set) {
    Message outMsg = {};
    outMsg.header.messageId  = (counter++); ///- request.header.messageId;
    outMsg.header.sessionId  = request.header.sessionId;
    outMsg.header.destNodeId = request.header.sourceNodeId;
    outMsg.decodedPayload.header.protocolCode = messageType;
    outMsg.decodedPayload.header.exchangeFlags.requiresAck = 0;
    outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    outMsg.decodedPayload.payload = set; // planned to replace by TLV encoding
    return outMsg;
}

//
Message Channel::makeMessage(Message const& request, uint8_t messageType, tlvcpp::tlv_tree_node const& TLV) {
    Message outMsg = {};
    outMsg.header.messageId  = (counter++); ///- request.header.messageId;
    outMsg.header.sessionId  = request.header.sessionId;
    outMsg.header.destNodeId = request.header.sourceNodeId;
    outMsg.decodedPayload.header.protocolCode = messageType;
    outMsg.decodedPayload.header.exchangeFlags.requiresAck = 0;
    outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
    outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
    outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
    outMsg.decodedPayload.TLV = TLV; // planned to replace by TLV encoding
    return outMsg;
}

//
#endif

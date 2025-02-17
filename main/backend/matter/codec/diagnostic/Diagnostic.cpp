#include "../message/Consts.hpp"
#include "./Diagnostic.hpp"
#include "../message/Message.hpp"

//
MsgTypeInfo Diagnostic::mapProtocolAndMessageType(uint32_t protocolId, uint8_t messageType) {
    std::string msgTypeHex = Diagnostic::hex(messageType);
    std::string type = Diagnostic::hex(protocolId) + "/" + msgTypeHex;
    if (protocolId == SECURE_CHANNEL_PROTOCOL_ID)
        { return { type, "SC/" + SecureMessageType::toString(messageType) }; }
    else if (protocolId == INTERACTION_PROTOCOL_ID)
        { return { type, "I/" + MessageType::toString(messageType) }; }
    return { type, "" };
}

//
std::string Diagnostic::messageDiagnostics(const Message& message, const std::string& logContext)
{
    auto& payload = message.decodedPayload;
    bool duplicate = false;
    auto info = mapProtocolAndMessageType(payload.header.protocolId, payload.header.messageType);
    std::vector<std::pair<std::string, std::string>> diag = {
        { "for", logContext },
        { "msgId",
            std::to_string(message.header.sessionId) + "/" +
            std::to_string(payload.header.exchangeId) + "/" +
            std::to_string(message.header.messageId) },
        { "type", info.type },
        { "acked", payload.header.ackedMessageId ? std::to_string(payload.header.ackedMessageId) : "N/A" },
        { "msgFlags", Diagnostic::asFlags(payload.header.requiresAck, duplicate) },
        { "size", !payload.payload->empty() ? std::to_string(payload.payload->size()) : "0" }
    };
    return Diagnostic::dict(diag);
}

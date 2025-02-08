//
#include "./Diagnostic.hpp"

//
static inline std::string Diagnostic::messageDiagnostics(const Message& message, const std::optional<std::string>& logContext = std::nullopt) 
{
    bool duplicate = false; // для примера
    auto info = mapProtocolAndMessageType(message.payloadHeader.protocolId, message.payloadHeader.messageType);
    std::vector<std::pair<std::string, std::string>> diag = {
        { "for", logContext.value_or(info.forType.value_or("")) },
        { "msgId", 
            std::to_string(message.packetHeader.sessionId) + "/" + 
            std::to_string(message.payloadHeader.exchangeId) + "/" + 
            std::to_string(message.packetHeader.messageId) },
        { "type", info.type },
        { "acked", message.payloadHeader.ackedMessageId ? std::to_string(*message.payloadHeader.ackedMessageId) : "N/A" },
        { "msgFlags", Diagnostic::asFlags(message.payloadHeader.requiresAck, duplicate) },
        { "size", !message.payload.empty() ? std::to_string(message.payload.size()) : "0" }
    };
    return Diagnostic::dict(diag);
}

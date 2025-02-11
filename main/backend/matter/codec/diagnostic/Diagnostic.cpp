//
#include "./Diagnostic.hpp"
#include "../message/Message.hpp"

//
static inline std::string Diagnostic::messageDiagnostics(const Message& message, const std::string& logContext = "")
{
    bool duplicate = false;
    auto info = mapProtocolAndMessageType(message.payloadHeader.protocolId, message.payloadHeader.messageType);
    std::vector<std::pair<std::string, std::string>> diag = {
        { "for", logContext },
        { "msgId",
            std::to_string(message.packetHeader.sessionId) + "/" +
            std::to_string(message.payloadHeader.exchangeId) + "/" +
            std::to_string(message.packetHeader.messageId) },
        { "type", info.type },
        { "acked", message.payloadHeader.ackedMessageId ? std::to_string(message.payloadHeader.ackedMessageId) : "N/A" },
        { "msgFlags", Diagnostic::asFlags(message.payloadHeader.requiresAck, duplicate) },
        { "size", !message.payload.empty() ? std::to_string(message.payload.size()) : "0" }
    };
    return Diagnostic::dict(diag);
}

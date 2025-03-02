#include "../message/Consts.hpp"
#include "../message/Message.hpp"
#include "./Diagnostic.hpp"

//
#include <iostream>
#include <optional>
#include <string>



//
namespace Diagnostic {
    constexpr uint32_t SECURE_CHANNEL_PROTOCOL_ID = 0x0001;
    constexpr uint32_t INTERACTION_PROTOCOL_ID    = 0x0002;

    //
    MsgTypeInfo mapProtocolAndMessageType(uint32_t const& protocolId, uint8_t const& messageType) {
        std::string msgTypeHex = Diagnostic::hex(messageType);
        std::string type = Diagnostic::hex(protocolId) + "/" + msgTypeHex;
        if (protocolId == SECURE_CHANNEL_PROTOCOL_ID) { return { type, "SC/" + SecureMessageType::toString(messageType) }; } else
        if (protocolId == INTERACTION_PROTOCOL_ID)    { return { type, "I/"  + MessageType::toString(messageType) }; }
        return { type, "" };
    }

    //
    std::string messageDiagnostics(Message const&  message, std::string const&  logContext) {
        auto& payload = message.decodedPayload;
        bool duplicate = false;
        auto info = mapProtocolAndMessageType(payload.header.protocolId, payload.header.messageType);
        std::vector<std::pair<std::string, std::string>> diag = {
            { "for", logContext },
            { "msgId", std::to_string(message.header.sessionId) + "/" + std::to_string(payload.header.exchangeId) + "/" + std::to_string(message.header.messageId) },
            { "type", info.type },
            { "acked", payload.header.ackedMessageId ? std::to_string(payload.header.ackedMessageId) : "N/A" },
            { "msgFlags", Diagnostic::asFlags(payload.header.messageFlags.requiresAck, duplicate) },
            { "size", !payload.payload->empty() ? std::to_string(payload.payload->size()) : "0" }
        };
        return Diagnostic::dict(diag);
    }
};



//
Payload const& MessageCodec::debugPayload(Payload const& payload) {
    std::cout << "Payload Debug:" << std::endl;
    std::cout << "  Message Flags:" << std::endl;
    std::cout << "    Is Initiator Message: " << std::to_string(payload.header.messageFlags.isInitiatorMessage) << std::endl;
    std::cout << "    Is Ack Message: " << std::to_string(payload.header.messageFlags.isAckMessage) << std::endl;
    std::cout << "    Requires Ack: " << std::to_string(payload.header.messageFlags.requiresAck) << std::endl;
    std::cout << "    Has Secure Extension: " << std::to_string(payload.header.messageFlags.hasSecureExtension) << std::endl;
    std::cout << "    Has Vendor ID: " << std::to_string(payload.header.messageFlags.hasVendorId) << std::endl;
    std::cout << "  Exchange ID: " << std::to_string(payload.header.exchangeId) << std::endl;
    std::cout << "  Protocol ID: " << std::to_string(payload.header.protocolId) << std::endl;
    std::cout << "  Acked Message ID: " << std::to_string(payload.header.ackedMessageId) << std::endl;
    std::cout << "  Vendor ID: " << std::to_string(payload.header.vendorId) << std::endl;
    std::cout << "  Message Type: " << (payload.header.messageFlags.hasSecureExtension ? Diagnostic::SecureMessageType::toString(payload.header.messageType) : Diagnostic::MessageType::toString(payload.header.messageType)) << std::endl;
    std::cout << "  Payload Size: " << payload.payload->size() << " bytes" << std::endl;
    std::cout << "  Security Extension Size: " << payload.securityExtension->size() << " bytes" << std::endl;
    return payload;
}

//
Message const& MessageCodec::debugMessage(Message const& message) {
    std::cout << "Message Debug:" << std::endl;
    std::cout << "  Message ID: " << message.header.messageId << std::endl;
    std::cout << "  Session ID: " << message.header.sessionId << std::endl;
    std::cout << "  Security Flags:" << std::endl;
    std::cout << "    Session Type: " << static_cast<int>(message.header.securityFlags.sessionType) << std::endl;
    std::cout << "    Unknown: " << static_cast<int>(message.header.securityFlags.unknown) << std::endl;
    std::cout << "    Has Message Extensions: " << static_cast<int>(message.header.securityFlags.hasMessageExtensions) << std::endl;
    std::cout << "    Is Control Message: " << static_cast<int>(message.header.securityFlags.isControlMessage) << std::endl;
    std::cout << "    Has Privacy Enhancements: " << static_cast<int>(message.header.securityFlags.hasPrivacyEnhancements) << std::endl;
    std::cout << "  Exchange Flags:" << std::endl;
    std::cout << "    Has Dest Node ID: " << static_cast<int>(message.header.exchangeFlags.hasDestNodeId) << std::endl;
    std::cout << "    Has Dest Group ID: " << static_cast<int>(message.header.exchangeFlags.hasDestGroupId) << std::endl;
    std::cout << "    Has Source Node ID: " << static_cast<int>(message.header.exchangeFlags.hasSourceNodeId) << std::endl;
    std::cout << "    Version: " << static_cast<int>(message.header.exchangeFlags.version) << std::endl;
    std::cout << "  Dest Node ID: " << (message.header.destNodeId ? std::to_string(*message.header.destNodeId) : "None") << std::endl;
    std::cout << "  Dest Group ID: " << (message.header.destGroupId ? std::to_string(*message.header.destGroupId) : "None") << std::endl;
    std::cout << "  Source Node ID: " << (message.header.sourceNodeId ? std::to_string(*message.header.sourceNodeId) : "None") << std::endl;
    std::cout << "  Raw Payload Size: " << message.rawPayload->size() << " bytes" << std::endl;
    std::cout << "  Message Extension Size: " << message.messageExtension->size() << " bytes" << std::endl;
    return message;
}

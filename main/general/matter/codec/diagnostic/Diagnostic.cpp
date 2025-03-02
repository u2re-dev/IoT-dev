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
        auto info = mapProtocolAndMessageType(payload.header.protocolId, payload.header.protocolOpCode);
        std::vector<std::pair<std::string, std::string>> diag = {
            { "for", logContext },
            { "msgId", std::to_string(message.header.sessionId) + "/" + std::to_string(payload.header.exchangeId) + "/" + std::to_string(message.header.messageId) },
            { "type", info.type },
            { "acked", payload.header.ackedMessageId ? std::to_string(payload.header.ackedMessageId) : "N/A" },
            { "msgFlags", Diagnostic::asFlags(payload.header.exchangeFlags.requiresAck, duplicate) },
            { "size", !payload.payload->empty() ? std::to_string(payload.payload->size()) : "0" }
        };
        return Diagnostic::dict(diag);
    }
};

//
inline std::string as_hex(auto const& v) {
    std::ostringstream oss;
    oss << std::hex << std::setw(sizeof(v) * 2) << std::setfill('0') << (int64_t)v;
    return "0x" + oss.str();
}

//
Message const& MessageCodec::debugMessage(Message const& message) {
    std::cout << "Message Debug:" << std::endl;
    std::cout << "  Message ID: " << as_hex(message.header.messageId) << std::endl;
    std::cout << "  Session ID: " << as_hex(message.header.sessionId) << std::endl;
    std::cout << "  Security Flags:" << std::endl;
    std::cout << "    Session Type: " << std::to_string(message.header.securityFlags.sessionType) << std::endl;
    std::cout << "    Unknown: " << std::to_string(message.header.securityFlags.unknown) << std::endl;
    std::cout << "    Has Message Extensions: " << std::to_string(message.header.securityFlags.hasMessageExtensions) << std::endl;
    std::cout << "    Is Control Message: " << std::to_string(message.header.securityFlags.isControlMessage) << std::endl;
    std::cout << "    Has Privacy Enhancements: " << std::to_string(message.header.securityFlags.hasPrivacyEnhancements) << std::endl;
    std::cout << "  Message Flags:" << std::endl;
    std::cout << "    Has Dest Node ID: " << std::to_string(message.header.messageFlags.hasDestNodeId) << std::endl;
    std::cout << "    Has Dest Group ID: " << std::to_string(message.header.messageFlags.hasDestGroupId) << std::endl;
    std::cout << "    Has Source Node ID: " << std::to_string(message.header.messageFlags.hasSourceNodeId) << std::endl;
    std::cout << "    Version: " << std::to_string(message.header.messageFlags.version) << std::endl;
    std::cout << "  Dest Node ID: " << (message.header.destNodeId ? as_hex(*message.header.destNodeId) : "None") << std::endl;
    std::cout << "  Dest Group ID: " << (message.header.destGroupId ? as_hex(*message.header.destGroupId) : "None") << std::endl;
    std::cout << "  Source Node ID: " << (message.header.sourceNodeId ? as_hex(*message.header.sourceNodeId) : "None") << std::endl;
    std::cout << "  Raw Payload Size: " << message.rawPayload->size() << " bytes" << std::endl;
    std::cout << "  Message Extension Size: " << message.messageExtension->size() << " bytes" << std::endl;
    return message;
}

//
Payload const& MessageCodec::debugPayload(Payload const& payload) {
    std::cout << "Payload Debug:" << std::endl;
    std::cout << "  Exchange Flags:" << std::endl;
    std::cout << "    Is Initiator Message: " << std::to_string(payload.header.exchangeFlags.isInitiatorMessage) << std::endl;
    std::cout << "    Is Ack Message: " << std::to_string(payload.header.exchangeFlags.isAckMessage) << std::endl;
    std::cout << "    Requires Ack: " << std::to_string(payload.header.exchangeFlags.requiresAck) << std::endl;
    std::cout << "    Has Secure Extension: " << std::to_string(payload.header.exchangeFlags.hasSecureExtension) << std::endl;
    std::cout << "    Has Vendor ID: " << std::to_string(payload.header.exchangeFlags.hasVendorId) << std::endl;
    std::cout << "  Exchange ID: " << as_hex(payload.header.exchangeId) << std::endl;
    std::cout << "  Protocol ID: " << as_hex(payload.header.protocolId) << std::endl;
    std::cout << "  Acked Message ID: " << as_hex(payload.header.ackedMessageId) << std::endl;
    std::cout << "  Vendor ID: " << as_hex(payload.header.vendorId) << std::endl;
    std::cout << "  Protocol OpCode: " << as_hex(payload.header.protocolOpCode) << std::endl;
    std::cout << "  Payload Size: " << payload.payload->size() << " bytes" << std::endl;
    std::cout << "  Security Extension Size: " << payload.securityExtension->size() << " bytes" << std::endl;
    return payload;
}

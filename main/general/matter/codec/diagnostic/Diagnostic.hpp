#pragma once

//
#include "../message/Message.hpp"
#include <sstream>

//
#pragma pack(push, 1)
struct NotImplementedError : public std::logic_error   { using std::logic_error::logic_error; };
struct UnexpectedDataError : public std::runtime_error { using std::runtime_error::runtime_error; };
#pragma pack(pop)

//
namespace Diagnostic {
#pragma pack(push, 1)
    struct MsgTypeInfo { std::string type = "", forType = ""; };
    struct SecureMessageType { static inline std::string toString(uint8_t msgType) { return std::to_string(msgType) + " (Has secure)"; } };
    struct MessageType { static inline std::string toString(uint8_t msgType) { return std::to_string(msgType) + " (No secure)"; } };
#pragma pack(pop)

    //
    inline MsgTypeInfo mapProtocolAndMessageType(uint32_t const& protocolId, uint8_t const& messageType);
    inline std::string asFlags(bool const& reqAck, bool const& duplicate) { return std::string("{ reqAck: ") + (reqAck ? "true" : "false") + ", dup: " + (duplicate ? "true" : "false") + " }"; }
    inline std::string messageDiagnostics(const Message& message, std::string const& logContext = "");
    inline std::string hex(uint32_t const& val) { std::ostringstream oss; oss << std::hex << val; return oss.str(); }
    inline std::string dict(std::vector<std::pair<std::string, std::string>> const& entries) { std::ostringstream oss; oss << "{ "; for (const auto& [key, value] : entries) oss << key << ": " << value << ", "; oss << "}"; return oss.str(); }
}

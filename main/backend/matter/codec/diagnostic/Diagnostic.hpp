#pragma once

//
#include "../core/STD.hpp"
#include "../message/Message.hpp"

//
struct NotImplementedError : public std::logic_error   { using std::logic_error::logic_error; };
struct UnexpectedDataError : public std::runtime_error { using std::runtime_error::runtime_error; };

//
namespace Diagnostic {
    inline std::string messageDiagnostics(const Message& message, std::string const& logContext = "");
    inline std::string hex(uint32_t val) {
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    }

    inline std::string asFlags(bool reqAck, bool duplicate) {
        return std::string("{ reqAck: ") + (reqAck ? "true" : "false") + ", dup: " + (duplicate ? "true" : "false") + " }";
    }

    inline std::string dict(const std::vector<std::pair<std::string, std::string>>& entries) {
        std::ostringstream oss;
        oss << "{ ";
        for (const auto& [key, value] : entries) oss << key << ": " << value << ", ";
        oss << "}";
        return oss.str();
    }
}

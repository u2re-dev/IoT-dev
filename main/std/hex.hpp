
#pragma once
#include "types.hpp"

//
namespace hex {
    
    //
    inline bytes_t s2b(const std::string& str) {
        return bytes_t(str.begin(), str.end());
    }

    //
    /*inline std::string b2h(uint8_t const* bytes, size_t len) {
        std::ostringstream oss;
        for (int i = len - 1; i >= 0; i--) { // Обратный цикл
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
        }
        return oss.str();
    }*/

    //
    inline std::string b2h(uint8_t const* bytes, size_t len) {
        std::ostringstream oss;
        //for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        for (uint i=0;i<len;i++) (oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i]);
        return oss.str();
    }

    //
    inline std::string b2h(const bytes_t& bytes) {
        std::ostringstream oss;
        for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        //for (uint i=0;i<bytes.size();i++) (oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[bytes.size() - 1 - i]);
        return oss.str();
    }

    //
    inline bytes_t h2b(const std::string& hex) {
        if (hex.size() % 2 != 0) throw std::invalid_argument("hex invalid");
        bytes_t array; //array.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            array.push_back(std::stoul(hex.substr(i, 2), nullptr, 16));
        }
        //std::reverse(array.begin(), array.end());
        return array;//.reverse();
    }

}

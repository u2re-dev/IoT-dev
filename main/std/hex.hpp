
#pragma once

//
#include "types.hpp"
#include <sstream>
#include <iomanip>

//
namespace hex {
    
    inline bytespan_t n2b(bigint_t const &x) {
        auto a = make_bytes(sizeof(x)); memcpy(a->data(), &x, sizeof(x)); return a;
    }

    //
    inline bytespan_t s2b(const std::string& str) {
        return make_bytes(str.begin(), str.end());
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
    inline std::string b2h(const bytespan_t& bytes) {
        std::ostringstream oss;
        //for (auto b : (*bytes)) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        for (uint i=0;i<bytes->size();i++) (oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i]);
        return oss.str();
    }

    //
    inline bytespan_t h2b(const std::string& hex) {
        if (hex.size() % 2 != 0) throw std::invalid_argument("hex invalid");
        auto array = make_bytes(); array->reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            array->push_back(std::stoul(hex.substr(i, 2), nullptr, 16));
        }
        //std::reverse(array.begin(), array.end());
        return array;//.reverse();
    }

}

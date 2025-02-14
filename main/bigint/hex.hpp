
#pragma once
#include "../std/types.hpp"

//
namespace hex {

    //
    inline bigint_t b2n_be(const uint8_t* b, size_t size = 32) {
        bigint_t x;
        std::memcpy(&x, b, sizeof(x));
        //return intx::to_little_endian(x);
        return intx::to_big_endian(x);
    }

    //
    inline bigint_t b2n(const uint8_t* b, size_t size = 32) {
        bigint_t x;
        std::memcpy(&x, b, sizeof(x));
        //return intx::to_little_endian(x);
        return intx::to_little_endian(x);
    }

    //
    inline void n2b(bigint_t num, uint8_t* b, size_t size = 32) {
        //const auto d = intx::to_little_endian(num);
        const auto d = intx::to_little_endian(num);
        std::memcpy(b, &d, sizeof(d));
    }

    //
    inline void n2b_be(bigint_t num, uint8_t* b, size_t size = 32) {
        const auto d = intx::to_big_endian(num);
        std::memcpy(b, &d, sizeof(d));
    }






    //
    inline std::string n2h(bigint_t const& num) {
        return intx::to_string(intx::to_little_endian(num), 16);//.substr(2);
    }

    //
    inline bigint_t h2n(const std::string& hex) {
        return intx::to_little_endian(intx::from_string<bigint_t>("0x"+hex)); //b2n(h2b(hex));
    }

    //
    inline bigint_t h2n(char const *hex, size_t len) {
        return intx::to_little_endian(intx::from_string<bigint_t>("0x"+std::string(hex)));
    }



    //
    inline bigint_t h2n_be(const std::string& hex) {
        return intx::to_big_endian(intx::from_string<bigint_t>("0x"+hex)); //b2n(h2b(hex));
    }




    //
    inline bytes_t s2b(const std::string& str) {
        return bytes_t(str.begin(), str.end());
    }

    //
    inline bytes_t n2b_be(bigint_t const& num, size_t size = 32) {
        bytes_t bytes(size, 0);
        n2b_be(num, bytes.data(), size);
        return bytes;
    }

    //
    inline bigint_t b2n(bytes_t const& b) {
        return b2n(b.data(), b.size());
    }

    //
    inline bigint_t b2n_be(bytes_t const& b) {
        return b2n_be(b.data(), b.size());
    }
    
    //
    inline bytes_t n2b(bigint_t const& num, size_t byteLen = 32) {
        bytes_t bytes(byteLen, 0);
        n2b(num, bytes.data(), byteLen);
        return bytes;
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

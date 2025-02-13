
#pragma once
#include "./types.hpp"

//
namespace hex {

    /**
    * @brief Converts a vector of bytes (uint8_t) to a hexadecimal string.
    *
    * This function takes a vector of bytes (each byte being an unsigned 8-bit integer)
    * and converts it into a string representation where each byte is represented
    * by its hexadecimal equivalent. For example, a byte `0xB4` would be represented
    * as the string "B4". This is useful for displaying or storing binary data in
    * a human-readable hexadecimal format.
    *
    * @param bytes The vector of bytes to be converted to a hexadecimal string.
    * @return A std::string representing the hexadecimal value of the input bytes.
    *
    * Example Usage:
    * @code
    * std::vector<uint8_t> data = {0xBA, 0xAD, 0xF0, 0x0D};
    * std::string hexStr = bytesToHex(data);
    * // hexStr will be "BAADF00D"
    * @endcode
    */

    //
    inline std::string bytesToHex(const bytes_t& bytes) {
        std::ostringstream oss;
        for (auto b : bytes) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        return oss.str();
    }

    //
    inline bytes_t hexToBytes(const std::string& hex) {
        if (hex.size() % 2 != 0) throw std::invalid_argument("hex invalid");
        bytes_t array;
        array.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            array.push_back(byte);
        }
        return array;
    }




    //
    inline bigint_t bytesToBigint(const bytes_t& bytes) {
        // Простой пример: интерпретировать массив байт как число (big-endian).
        uint256_t res = 0;
        for (size_t i = 0; i < bytes.size(); i++) {
            res = (res << 8) | bytes[i];
        }
        return res;
    }

    // Функция для преобразования числа в массив байт в big-endian с фиксированной длиной.
    inline bytes_t numberToBytesBE(bigint_t num, size_t byteLen) {
        bytes_t bytes(byteLen, 0);
        for (size_t i = 0; i < byteLen; i++) {
            bytes[byteLen - 1 - i] = static_cast<uint8_t>(num & 0xff);
            num >>= 8;
        }
        return bytes;
    }

    // Функция для преобразования строки в bytes_t (ASCII)
    inline bytes_t stringToBytes(const std::string& str) {
        return bytes_t(str.begin(), str.end());
    }



    //
    inline std::string n2h(const bigint_t &num) {
        std::ostringstream oss;
        oss << std::hex << num;
        std::string s = oss.str();
        if(s.size() < 64) s = std::string(64 - s.size(), '0') + s;
        return s;
    }

    //
    inline bigint_t b2n(const uint8_t* b) {
        bigint_t res = 0;
        //for (uint i=0;i<32;i++) res |= (b[i] << 8); // LE
          for (uint i=0;i<32;i++) res  = (res  << 8) | b[i]; // BE
        return res;
    }

    //
    inline bigint_t b2n(const bytes_t &b) {
        bigint_t res = 0;
        //for (auto byte : b) res |= (byte << 8); // LE
          for (auto byte : b) res  = (res  << 8) | byte; // BE
        return res;
    }

    //
    inline void n2b(bigint_t num, uint8_t* buffer, size_t size = 32) {
        for (size_t i = 0; i < size; ++i) {
            buffer[size - 1 - i] = static_cast<uint8_t>(num >> (i * 8));
        }
    }
}

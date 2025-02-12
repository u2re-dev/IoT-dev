
#pragma once

//////////////////////////////////////////////////////////
// 2. Вспомогательные функции для hex‑строк и байтовых массивов
//////////////////////////////////////////////////////////
namespace HexUtil {

    // Функция для преобразования hex-строки в ByteArray.
    /*inline ByteArray hexToBytes(const std::string& hex) {
        if (hex.size() % 2 != 0) throw std::invalid_argument("Неверная длина hex-строки");
        ByteArray bytes;
        bytes.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }*/

    // Преобразование hex‑строки в массив байт.
    inline Bytes hexToBytes(const std::string& hex) {
        if(hex.size() % 2 != 0) throw std::invalid_argument("hex invalid");
        Bytes array;
        array.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            array.push_back(byte);
        }
        return array;
    }

    // Преобразование массива байт в hex‑строку.
    inline std::string bytesToHex(const Bytes& bytes) {
        std::ostringstream oss;
        for (auto b : bytes)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        return oss.str();
    }

    // =====================================================================
    // Пример служебных функций для конвертации
    // Функция для преобразования массива байт в число (big-endian).
    // Реализуйте корректно в зависимости от вашей реализации BigInt.
    inline uint256_t bytesToBigInt(const ByteArray& bytes) {
        // Простой пример: интерпретировать массив байт как число (big-endian).
        uint256_t res = 0;
        for (size_t i = 0; i < bytes.size(); i++) {
            res = (res << 8) | bytes[i];
        }
        return res;
    }

    // Функция для преобразования числа в массив байт в big-endian с фиксированной длиной.
    inline ByteArray numberToBytesBE(uint256_t num, size_t byteLen) {
        ByteArray bytes(byteLen, 0);
        for (size_t i = 0; i < byteLen; i++) {
            bytes[byteLen - 1 - i] = static_cast<uint8_t>(num & 0xff);
            num >>= 8;
        }
        return bytes;
    }

    // Функция для преобразования строки в ByteArray (ASCII)
    inline ByteArray stringToBytes(const std::string& str) {
        return ByteArray(str.begin(), str.end());
    }

} // namespace HexUtil

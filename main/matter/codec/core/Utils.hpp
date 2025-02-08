#pragma once
#include "./STD.hpp"

// 
namespace Bytes {
    inline ByteArray concat(const std::initializer_list<ByteArray>& arrays) {
        ByteArray result;
        for (const auto& arr : arrays)
            result.insert(result.end(), arr.begin(), arr.end());
        return result;
    }
}

//
class DataReader {
public:
    inline DataReader(const ByteArray& data_)
        : begin(data_.data()), ptr(data_.data()), end(data_.data() + data_.size()) {}

    inline uint8_t readUInt8() {
        checkSize(1);
        return *ptr++;
    }

    inline uint16_t readUInt16() {
        checkSize(2);
        uint16_t value = ptr[0] | (ptr[1] << 8);
        ptr += 2;
        return value;
    }

    inline uint32_t readUInt32() {
        checkSize(4);
        uint32_t value = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
        ptr += 4;
        return value;
    }

    inline uint64_t readUInt64() {
        checkSize(8);
        uint64_t value = 0;
        for (int i = 0; i < 8; ++i)
            value |= static_cast<uint64_t>(ptr[i]) << (8 * i);
        ptr += 8;
        return value;
    }

    inline ByteArray readByteArray(size_t length) {
        checkSize(length);
        ByteArray buf(ptr, ptr + length);
        ptr += length;
        return buf;
    }

    inline ByteArray remainingBytes() const {
        return ByteArray(ptr, end);
    }

private:
    inline void checkSize(size_t n) const {
        if (ptr + n > end) throw UnexpectedDataError("Unexpected end of data");
    }
    const Byte* begin;
    const Byte* ptr;
    const Byte* end;
};

// Упрощённая реализация DataWriter
class DataWriter {
public:
    inline void writeUInt8(uint8_t value) {
        data.push_back(value);
    }
    
    inline void writeUInt16(uint16_t value) {
        data.push_back(static_cast<Byte>(value & 0xff));
        data.push_back(static_cast<Byte>((value >> 8) & 0xff));
    }
    
    inline void writeUInt32(uint32_t value) {
        for (int i = 0; i < 4; ++i)
            data.push_back(static_cast<Byte>((value >> (8*i)) & 0xff));
    }
    
    inline void writeUInt64(uint64_t value) {
        for (int i = 0; i < 8; ++i) 
            data.push_back(static_cast<Byte>((value >> (8*i)) & 0xff));
    }
    
    inline void writeByteArray(const ByteArray& arr) {
        data.insert(data.end(), arr.begin(), arr.end());
    }
    
    inline ByteArray toByteArray() const {
        return data;
    }
private:
    ByteArray data;
};

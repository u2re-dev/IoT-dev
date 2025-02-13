#pragma once

// C++ libs
#include <optional>
#include <string>
#include <array>
#include <stdexcept>
#include <sstream>
#include <concepts>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <utility>
#include <functional>
#include <atomic>
#include <thread>
#include <tuple>
#include <random>

// C-libs
#include <cmath>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>

//
#include "./uint128/uint128_t.h"
#include "./uint256/uint256_t.h"

//
using bigint_t = uint256_t;
using bytes_t  = std::vector<uint8_t>;
using byte_t   = uint8_t;
using node_id  = uint64_t;
using group_id = uint16_t;

//
struct affine_t { bigint_t x; bigint_t y; };

//
inline bytes_t concat(const std::initializer_list<bytes_t>& arrays) {
    bytes_t result;
    for (const auto& arr : arrays) result.insert(result.end(), arr.begin(), arr.end());
    return result;
}

//
class DataReader {
public:
    inline DataReader(bytes_t const& data_) : begin(data_.data()), ptr(data_.data()), end(data_.data() + data_.size()) {}
    inline DataReader(uint8_t const* data_) : begin(data_), ptr(data_), end(nullptr) {}


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

    inline uint8_t const* makeSlice(size_t length) {
        checkSize(length);
        uint8_t const* point = ptr;
        ptr += length;
        return point;
    }

    inline bytes_t readByteArray(size_t length) {
        checkSize(length);
        bytes_t buf(ptr, ptr + length);
        ptr += length;
        return buf;
    }

    inline bytes_t remainingBytes() const {
        return bytes_t(ptr, end);
    }

private:
    inline void checkSize(size_t n) const {
        if (end && (ptr + n > end)) throw "Unexpected end of data";
    }

    //
    const byte_t* begin = nullptr;
    const byte_t* ptr   = nullptr;
    const byte_t* end   = nullptr;
};

//
class DataWriter {
public:
    inline void writeUInt8(uint8_t value) {
        data.push_back(value);
    }

    inline void writeUInt16(uint16_t value) {
        data.push_back(static_cast<byte_t>(value & 0xff));
        data.push_back(static_cast<byte_t>((value >> 8) & 0xff));
    }

    inline void writeUInt32(uint32_t value) {
        for (int i = 0; i < 4; ++i) {
            data.push_back(static_cast<byte_t>((value >> (8*i)) & 0xff));
        }
    }

    inline void writeUInt64(uint64_t value) {
        for (int i = 0; i < 8; ++i) {
            data.push_back(static_cast<byte_t>((value >> (8*i)) & 0xff));
        }
    }

    inline void writeBytes(const bytes_t& arr) {
        data.insert(data.end(), arr.begin(), arr.end());
    }

    inline bytes_t toBytes() const {
        return data;
    }

private: bytes_t data;
};

//
class DataWriterLL {
public:
    DataWriterLL(uint8_t* buffer, size_t bufferSize) :
    ptr(buffer), data(buffer), capacity(bufferSize) {};

    inline void writeUInt8(uint8_t value) {
        ensureCapacity(1);
        *ptr = value; ptr++;
    }

    inline void writeUInt16(uint16_t value) {
        ensureCapacity(2);
        for (int i = 0; i < 2; ++i)
            { *ptr = static_cast<uint8_t>((value >> (8 * i)) & 0xff); ptr++; };
    }

    inline void writeUInt32(uint32_t value) {
        ensureCapacity(4);
        for (int i = 0; i < 4; ++i)
            { *ptr = static_cast<uint8_t>((value >> (8 * i)) & 0xff); ptr++; };
    }

    inline void writeUInt64(uint64_t value) {
        ensureCapacity(8);
        for (int i = 0; i < 8; ++i)
            { *ptr = static_cast<uint8_t>((value >> (8 * i)) & 0xff); ptr++; };
    }

    inline void writeBytes(const uint8_t* arr, size_t length) {
        ensureCapacity(length);
        memcpy(ptr, arr, length);
        ptr += length;
    }

    // Возвращает текущую длину записанных данных
    inline size_t getSize() const {
        return (ptr - data);
    }

private:
    uint8_t* data = nullptr;
    uint8_t* ptr  = nullptr;
    size_t   capacity  =  0;

    // Проверяет, достаточно ли места в буфере для записи
    void ensureCapacity(size_t additionalBytes) {
        if (capacity && ((ptr - data) > (capacity))) {
            throw std::runtime_error("Buffer overflow: not enough space in the buffer");
        }
    }
};

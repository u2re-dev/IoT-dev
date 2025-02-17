#pragma once

// C++ libs
#include <stdexcept>
#include <vector>

// C-libs
#include <cmath>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <memory>

//
#include "../spake2p/bigint/intx.hpp"

//
using byte_t   = uint8_t;
using bytes_t  = std::shared_ptr<std::vector<byte_t>>;
using node_id  = uint64_t;
using group_id = uint16_t;
using bigint_t = intx::uint256;

//
template<typename... Args>
inline bytes_t make_bytes(Args... args) {
    return std::make_shared<std::vector<byte_t>>(args...);
}

//
inline bytes_t concat(const std::initializer_list<bytes_t>& arrays) {
    bytes_t result = make_bytes();
    for (const auto& arr : arrays) result->insert(result->end(), arr->begin(), arr->end());
    return result;
}

//
class reader_t {
public:
    inline reader_t(bytes_t const& data_) : begin(data_->data()), ptr(data_->data()), end(data_->data() + data_->size()) {}
    inline reader_t(uint8_t const* data_) : begin(data_), ptr(data_), end(nullptr) {}


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
        bytes_t buf = make_bytes(ptr, ptr + length);
        ptr += length;
        return buf;
    }

    inline bytes_t remainingBytes() const {
        return make_bytes(ptr, end);
    }

    //
    inline operator bytes_t() const {
        return make_bytes(begin, end);
    }

private:
    inline void checkSize(size_t n) const {
        if (end && (ptr + n > end)) throw "Unexpected end of data";
    }

    //
    byte_t const* begin = nullptr;
    byte_t const* ptr   = nullptr;
    byte_t const* end   = nullptr;
};

//
class writer_t {
public:
    operator bytes_t&() { return data; };
    operator bytes_t const&() const { return data; };

    //
    writer_t() : data(make_bytes()) {}
    writer_t(bytes_t const& exists) : data(exists) {}

    //
    inline void writeUInt8(uint8_t value) {
        data->push_back(value);
    }

    //
    inline void writeUInt16(uint16_t value) {
        data->push_back(static_cast<byte_t>(value & 0xff));
        data->push_back(static_cast<byte_t>((value >> 8) & 0xff));
    }

    inline void writeUInt32(uint32_t value) {
        for (size_t i = 0; i < 4; ++i) {
            data->push_back(static_cast<byte_t>((value >> (8*i)) & 0xff));
        }
    }

    inline void writeUInt64(uint64_t value) {
        for (size_t i = 0; i < 8; ++i) {
            data->push_back(static_cast<byte_t>((value >> (8*i)) & 0xff));
        }
    }

    inline void writeBigint(const bigint_t& val) {
        for (size_t i = 0; i < 8; ++i) {
            data->push_back(byte_t((val >> (8*i)) & 0xff));
        }
    }

    // TODO: better method for writing bytes
    inline void writeBytes(const bytes_t& val) {
        for (size_t i = 0; i < val->size(); ++i) {
            data->push_back((*val)[i]);
        }
    }

    inline bytes_t toBytes() const {
        return data;
    }

private: bytes_t data;
};

//
class writer_l {
public:
    writer_l(uint8_t* buffer, size_t bufferSize) : data(buffer), ptr(buffer), capacity(bufferSize) {};

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

    inline void writeBytes(bytes_t const& arr) {
        ensureCapacity(arr->size());
        memcpy(ptr, arr->data(), arr->size());
        ptr += arr->size();
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
        if (capacity && ((ptr - data) > (additionalBytes))) {
            throw std::runtime_error("Buffer overflow: not enough space in the buffer");
        }
    }
};

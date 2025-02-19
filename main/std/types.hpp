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
inline bytes_t concat(std::initializer_list<bytes_t> const& arrays) {
    bytes_t result = make_bytes();
    for (const auto& arr : arrays) result->insert(result->end(), arr->begin(), arr->end());
    return result;
}

//
class reader_t {
public:
    inline reader_t(reader_t const& reader) : offset(reader.offset), memory(reader.memory), capacity(reader.capacity) {}
    inline reader_t(uint8_t const* data = nullptr, size_t size = 0) : offset(0), memory(data), capacity(size) {}
    inline reader_t(bytes_t const& data_) : memory(data_->data()), capacity(data_->size()) {}

    //
    inline bool checkMemory(size_t size = 1) const { return (capacity >= (size + offset)); }

    //
    inline int64_t& readInt64() { return *(int64_t*)allocate(8); }
    inline int32_t& readInt32() { return *(int32_t*)allocate(4); }
    inline int16_t& readInt16() { return *(int16_t*)allocate(2); }
    inline int8_t& readInt8 () { return *(int8_t *)allocate(1); }

    //
    inline uint64_t& readUInt64() { return *(uint64_t*)allocate(8); }
    inline uint32_t& readUInt32() { return *(uint32_t*)allocate(4); }
    inline uint16_t& readUInt16() { return *(uint16_t*)allocate(2); }
    inline uint8_t& readUInt8 () { return *(uint8_t *)allocate(1); }

    //
    inline byte_t& readByte () { return *(byte_t *)allocate(1); }

    //
    inline uint8_t const* allocate(size_t size) {
        if (capacity < (offset + size)) {
            throw std::runtime_error("Remain memory exceeded");
            return nullptr;
        }
        auto ptr = memory + offset;
        offset += size;
        return ptr;
    }

    //
    inline bytes_t readBytes(size_t length) {
        checkSize(length);
        auto bytes = allocate(length);
        return make_bytes(bytes, bytes + length);
    }

    //
    inline bytes_t remainingBytes() const {
        return make_bytes(memory + offset, memory + capacity);
    }

    //
    inline operator bytes_t() const {
        return make_bytes(memory, memory + offset);
    }

private:
    inline void checkSize(size_t n) const {
        if ((offset + n) < capacity) throw "Unexpected end of data";
    }

    //
    uintptr_t offset = 0;
    byte_t const* memory = nullptr;
    size_t capacity = 0;
};

//
class writer_t {
public:
    operator bytes_t&() { return data; };
    operator bytes_t const&() const { return data; };

    //
    inline writer_t() : data(make_bytes()) {}
    inline writer_t(bytes_t const& exists) : data(exists) {}
    inline writer_t(uint8_t* mem, size_t length) : data(make_bytes(mem, mem + length)) {}

    //
    inline uintptr_t offset() const {
        return data->size();
    }

    //
    inline writer_t& writeByte(uint8_t const& value) {
        data->push_back(value);
        return *this;
    }

    //
    inline writer_t& writeUInt8(uint8_t const& value) {
        data->push_back(value);
        return *this;
    }

    //
    inline writer_t& writeUInt16(uint16_t const& value) {
        data->reserve(2);
        data->push_back(static_cast<byte_t>(value & 0xff));
        data->push_back(static_cast<byte_t>((value >> 8) & 0xff));
        return *this;
    }

    inline writer_t& writeUInt32(uint32_t const& value) {
        data->reserve(4);
        for (size_t i = 0; i < 4; ++i) {
            data->push_back(static_cast<byte_t>((value >> (8*i)) & 0xff));
        }
        return *this;
    }

    inline writer_t& writeUInt64(uint64_t const& value) {
        data->reserve(8);
        for (size_t i = 0; i < 8; ++i) {
            data->push_back(static_cast<byte_t>((value >> (8*i)) & 0xff));
        }
        return *this;
    }

    inline writer_t& writeUInt128(intx::uint128 const& val) {
        data->reserve(16);
        for (size_t i = 0; i < 16; ++i) {
            data->push_back(byte_t((val >> (8*i)) & 0xff));
        }
        return *this;
    }

    inline writer_t& writeBigIntBE(bigint_t const& val) {
        data->reserve(32);
        for (size_t i = 0; i < 32; ++i) {
            data->push_back(byte_t((val >> (8*(31-i))) & 0xff));
        }
        return *this;
    }

    inline writer_t& writeBigInt(bigint_t const& val) {
        data->reserve(32);
        for (size_t i = 0; i < 32; ++i) {
            data->push_back(byte_t((val >> (8*i)) & 0xff));
        }
        return *this;
    }

    // TODO: better method for writing bytes
    inline writer_t& writeBytes(uint8_t const* val, size_t length) {
        data->reserve(length);
        for (size_t i = 0; i < length; ++i) {
            data->push_back(val[i]);
        }
        return *this;
    }

    // TODO: better method for writing bytes
    inline writer_t& writeBytes(bytes_t const& val) {
        data->reserve(val->size());
        for (size_t i = 0; i < val->size(); ++i) {
            data->push_back((*val)[i]);
        }
        return *this;
    }

    //
    inline bytes_t const& toBytes() const {
        return data;
    }

private: bytes_t data;
};

//
class writer_l {
public:
    inline writer_l(uint8_t* buffer = nullptr, size_t capacity = 0) : offset(0), data(buffer), capacity(capacity) {};

    //
    inline bool checkMemory(size_t size = 1) const { return ((offset + size) < capacity) || (!capacity); }

    //
    inline writer_l& writeUInt8(uint8_t value) {
        ensureCapacity(1);
        *(data + offset) = value; offset++;
        return *this;
    }

    inline writer_l& writeUInt16(uint16_t value) {
        ensureCapacity(2);
        *(uint16_t*)(data + offset) = value; offset += 2;
        return *this;
    }

    inline writer_l& writeUInt32(uint32_t value) {
        ensureCapacity(4);
        *(uint32_t*)(data + offset) = value; offset += 4;
        return *this;
    }

    inline writer_l& writeUInt64(uint64_t value) {
        ensureCapacity(8);
        *(uint64_t*)(data + offset) = value; offset += 8;
        return *this;
    }

    inline writer_l& writeBytes(bytes_t const& arr) {
        ensureCapacity(arr->size());
        memcpy(data + offset, arr->data(), arr->size());
        offset += arr->size();
        return *this;
    }

    inline writer_l& writeBytes(const uint8_t* arr, size_t length) {
        ensureCapacity(length);
        memcpy(data + offset, arr, length);
        offset += length;
        return *this;
    }

    //
    inline uintptr_t const& getSize() const { return offset; }

    //
    inline operator bytes_t() const {
        return toBytes();
    }

    //
    inline bytes_t toBytes() const {
        return make_bytes(data, data + offset);
    }

private:
    uint8_t* data = nullptr;
    uintptr_t offset = 0;
    size_t  capacity = 0;

    // Проверяет, достаточно ли места в буфере для записи
    void ensureCapacity(size_t additionalBytes) {
        if (!checkMemory()) {
            throw std::runtime_error("Buffer overflow: not enough space in the buffer");
        }
    }
};

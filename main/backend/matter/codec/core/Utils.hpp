#pragma once
#include "./STD.hpp"
#include "./Types.hpp"
#include "../diagnostic/Diagnostic.hpp"

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
    inline DataReader(ByteArray const& data_) : begin(data_.data()), ptr(data_.data()), end(data_.data() + data_.size()) {}
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
        if (end && (ptr + n > end)) throw UnexpectedDataError("Unexpected end of data");
    }

    //
    const Byte* begin = nullptr;
    const Byte* ptr   = nullptr;
    const Byte* end   = nullptr;
};


//
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

    // Запись массива байт: сначала длина, затем сами данные
    //inline void writeByteArray(const ByteArray& data) {
        //writeUInt64(data.size());
        //buffer.insert(buffer.end(), data.begin(), data.end());
    //}

    inline void writeByteArray(const ByteArray& arr) {
        data.insert(data.end(), arr.begin(), arr.end());
    }

    inline ByteArray toByteArray() const {
        return data;
    }

private:
    ByteArray data;
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

    inline void writeByteArray(const uint8_t* arr, size_t length) {
        ensureCapacity(length);
        memcpy(ptr, arr, length);
        ptr += length;
    }

    // Возвращает указатель на начало данных
    /*inline const uint8_t* toByteArray() const {
        return data;
    }*/

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

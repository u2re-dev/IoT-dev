#pragma once

//
#include <cstring>
#include <cstdint>
#include <ostream>
#include <vector>
#include <string>

//
#include "./parts/enums.hpp"

//
namespace tlvcpp
{
    // arven-code (8+8 bits)
    using type_t   = uint8_t;
    using tag_t    = uint8_t;

    //
    using length_t = uint32_t;

    //
    //using tag_parser = const char* (*)(const tag_t);
    //void set_tag_parser(tag_parser parser);
    //bool tag_is_primitive(tag_t tag);

    //
    class tlv {
    public:
        //
        tlv() {};
        explicit tlv(const tag_t& tag, size_t size = 0, const uint8_t* payload = nullptr, type_t type = 0) : m_tag(tag), m_size(size), m_payload(payload), m_type(type) {}

        //
        tlv(const tag_t& tag, const char* str) : m_tag(tag), m_size(std::strlen(str)), m_payload(reinterpret_cast<const uint8_t*>(str)), m_type(e_type::UTF8_STRING) {}
        tlv(const tag_t& tag, const std::string& str) : m_tag(tag), m_size(str.size()), m_payload(reinterpret_cast<const uint8_t*>(str.c_str())), m_type(e_type::UTF8_STRING) {}

        //
        explicit tlv(const tag_t& tag, uint32_t value) : m_tag(tag), m_u32(value), m_type(e_type::UNSIGNED_INTEGER | 0b00000010) {}
        explicit tlv(const tag_t& tag, uint16_t value) : m_tag(tag), m_u16(value), m_type(e_type::UNSIGNED_INTEGER | 0b00000001) {}
        explicit tlv(const tag_t& tag, uint8_t value)  : m_tag(tag), m_u8(value), m_type(e_type::UNSIGNED_INTEGER) {}

        //
        template <typename T> tlv(const tag_t& tag, T&& value);

        //
        tlv(const tlv& other);
        tlv(tlv&& other) noexcept;
        ~tlv();

        //
        tlv& operator=(const tlv& other);
        tlv& operator=(tlv&& other) noexcept;

        //
        bool operator==(tag_t const& other) const { return (other == m_tag); };
        bool operator==(tlv const& other) const { return (other.m_tag == m_tag); };
        //bool operator==(uint const& other) const { return (other == m_tag); };

        //
        friend std::ostream& operator<<(std::ostream& stream, const tlv& tlv_val);

        //
        type_t& type() { return m_type; }
        const type_t& type() const { return m_type; }
        tlv& type(const type_t& t) { m_type = t; return *this; }

        //
        tag_t& tag() { return m_tag; }
        const tag_t& tag() const { return m_tag; }
        tlv& tag(const tag_t& tg) { m_tag = tg; return *this; }

        //
        tlv& operator=(bool v) { m_b = v; return *this; }
        tlv& operator=(uint8_t v) { m_u8 = v; return *this; }
        tlv& operator=(uint16_t v) { m_u16 = v; return *this; }
        tlv& operator=(uint32_t v) { m_u32 = v; return *this; }
        tlv& operator=(uint64_t v) { m_u64 = v; return *this; }

        //
        tlv& setBytes(const uint8_t* bytes, size_t s) { m_size = s; m_payload = bytes; return *this; }

        //
        const size_t& size() const { return m_size; }
        const uint8_t* payload() const { return m_payload; }

        //
        operator std::vector<uint8_t>() const { return m_payload ? std::vector<uint8_t>(m_payload, m_payload + m_size) : std::vector<uint8_t>{}; }
        operator std::string() const { return m_payload ? std::string(reinterpret_cast<const char*>(m_payload), m_size) : ""; }

        //
        operator bool&() { return m_b; }
        operator const bool&() const { return m_b; }
        operator uint8_t&() { return m_u8; }
        operator const uint8_t&() const { return m_u8; }
        operator uint16_t&() { return m_u16; }
        operator const uint16_t&() const { return m_u16; }
        operator uint32_t&() { return m_u32; }
        operator const uint32_t&() const { return m_u32; }
        operator uint64_t&() { return m_u64; }
        operator const uint64_t&() const { return m_u64; }

    private:
        // arven-code (8+8 bits)
        type_t m_type = 0;
        tag_t m_tag = 0;

        // number value or size
        union {
            long m_value;
            size_t m_size;
            uint64_t m_u64;
            uint32_t m_u32;
            uint16_t m_u16;
            uint8_t  m_u8;
            bool     m_b;
        };

        // additional byte-stream or string (char-set)
        uint8_t const* m_payload = nullptr;
    };

}

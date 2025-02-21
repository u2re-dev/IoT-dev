#pragma once

//
#include <cstring>
#include <cstdint>
#include <ostream>
#include <vector>
#include <string>

//
#include "../std/types.hpp"
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
    struct control_t {
        uint8_t octet : 2;
        uint8_t type  : 3;
        uint8_t lab   : 3;
    };

    //
    class tlv {
    public:
        //
        tlv() {};

        //
        explicit tlv(control_t const& control, tag_t const& tag = 0) : m_tag(tag), m_control(control) { m_control.lab |= tag != 0 ? 0b001 : 0; }
        explicit tlv(size_t size, const uint8_t* payload, tag_t const& tag = 0) : m_tag(tag), m_size(size), m_payload(payload), m_control({ 0b00, e_type::BYTE_STRING, 0b00 }) { m_control.lab |= tag != 0 ? 0b001 : 0; }

        //
        explicit tlv(uint8_t const* str,     tag_t const& tag = 0) : m_tag(tag), m_size(std::strlen((char const* )str)), m_payload(reinterpret_cast<const uint8_t*>(str))        , m_control({ 0b00, e_type::UTF8_STRING, 0b00 }) { m_control.lab |= tag != 0 ? 0b001 : 0; }
        explicit tlv(std::string const& str, tag_t const& tag = 0) : m_tag(tag), m_size(str.size())                      , m_payload(reinterpret_cast<const uint8_t*>(str.c_str())) , m_control({ 0b00, e_type::BYTE_STRING, 0b00 }) { m_control.lab |= tag != 0 ? 0b001 : 0; }

        //
        tlv(bytespan_t const&  bn, tag_t const& tag = 0) : m_tag(tag), m_size(bn->size()), m_payload(reinterpret_cast<const uint8_t*>(bn->data())), m_control({ 0b00, e_type::BYTE_STRING, 0b00 }) { m_control.lab |= tag != 0 ? 0b001 : 0; }
        tlv(uint64_t const& value, tag_t const& tag = 0) : m_tag(tag), m_u64(value), m_control({0b11, e_type::UNSIGNED_INTEGER, 0b000}) { m_control.lab |= tag != 0 ? 0b001 : 0; }
        tlv(uint32_t const& value, tag_t const& tag = 0) : m_tag(tag), m_u32(value), m_control({0b10, e_type::UNSIGNED_INTEGER, 0b000}) { m_control.lab |= tag != 0 ? 0b001 : 0; }
        tlv(uint16_t const& value, tag_t const& tag = 0) : m_tag(tag), m_u16(value), m_control({0b01, e_type::UNSIGNED_INTEGER, 0b000}) { m_control.lab |= tag != 0 ? 0b001 : 0; }
        tlv( uint8_t const& value, tag_t const& tag = 0) : m_tag(tag),  m_u8(value), m_control({0b00, e_type::UNSIGNED_INTEGER, 0b000}) { m_control.lab |= tag != 0 ? 0b001 : 0; }

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
        type_t type() const { return m_control.type; }
        tlv& type(const type_t& t) { m_control.type = t; return *this; }

        //
        control_t& control() { return m_control; }
        const control_t& control() const { return m_control; }
        tlv& control(const control_t& ct) { m_control = ct; return *this; }

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
        control_t m_control = {0, 0, 0};
        tag_t m_tag = 0;

        // number value or size
        union {
            long m_value;
            size_t m_size;
            uint64_t m_u64 = 0;
            uint32_t m_u32;
            uint16_t m_u16;
            uint8_t  m_u8;
            bool     m_b;
        };

        // additional byte-stream or string (char-set)
        uint8_t const* m_payload = nullptr;
    };

}

#pragma once

//
#include <cstring>
#include <cstdint>
#include <ostream>
#include <vector>
#include <string>

//
namespace tlvcpp
{
    using type_t   = uint8_t;  // 
    using tag_t    = uint32_t; //
    using length_t = uint32_t; //
    using tag_parser = const char *(*)(const tag_t);

    //
    void set_tag_parser(tag_parser parser);
    bool tag_is_primitive(tag_t tag);

    //
    class tlv {
    public:
        tlv(const tag_t &tag, size_t const& size = 0, uint8_t const* && payload = nullptr, type_t type = 0) : m_tag(tag), m_size(size), m_payload(payload), m_type(type) {};
        tlv(const tag_t &tag, size_t const& size, uint8_t const* const& payload = nullptr, type_t type = 0) : m_tag(tag), m_size(size), m_payload(payload), m_type(type) {};
        tlv(const tag_t &tag, char const*string) : m_tag(tag), m_size(strlen(string)), m_payload((uint8_t const*)string), m_type(0x0C) {}

        //
        tlv(const tag_t &tag, std::string const& string) : 
            m_tag(tag), 
            m_size(string.size()), 
            m_payload((uint8_t const*)string.c_str()), 
            m_type(0x0C) {}

        //
        tlv(const tag_t &tag, uint32_t const& value) : m_tag(tag), m_u32(value), m_type(0x04) {}
        tlv(const tag_t &tag, uint16_t const& value) : m_tag(tag), m_u16(value), m_type(0x05) {}
        tlv(const tag_t &tag, uint8_t const& value) : m_tag(tag), m_u8(value), m_type(0x00) {}

        //
        template <typename T> tlv(const tag_t &tag, const T &&type);
        tlv() {};
        tlv(const tlv &other);
        tlv(tlv &&other) noexcept;
        ~tlv();

        //
        tlv &operator=(const tlv &other);
        tlv &operator=(tlv &&other) noexcept;

        //
        friend std::ostream &operator<<(std::ostream &stream, const tlv &tlv);
        friend bool operator==(const tlv &tlv, const tag_t &tag);

        //
        type_t& type() { return m_type; }
        type_t const& type() const { return m_type; }
        tlv& type(type_t const& type) { m_type = type; return *this; };

        //
        tag_t& tag() { return m_tag; }
        tag_t const& tag() const { return m_tag; }
        tlv& tag(tag_t const& tag) { m_tag = tag; return *this; };

        //
        tlv& operator=(bool const& v) { m_b = v; return *this; };
        tlv& operator=(uint8_t const& v) { m_u8 = v; return *this; };
        tlv& operator=(uint16_t const& v) { m_u16 = v; return *this; };
        tlv& operator=(uint32_t const& v) { m_u32 = v; return *this; }; //

        //
        tlv& setBytes(uint8_t const* bytes, size_t s) { m_size = s; m_payload = bytes; return *this; };
        size_t const& size() const { return m_size; };
        uint8_t const* payload() const { return m_payload; };

        //const value_t *value() const { return &m_value; }

        //
        //operator value_t& () { return m_value; }
        //operator value_t const& () const { return m_value; }

        //
        operator std::vector<uint8_t>() const { return m_payload ? std::vector<uint8_t>(m_payload, m_payload + m_size) : std::vector<uint8_t>{}; }
        operator std::string() const { return m_payload ? std::string(m_payload, m_payload + m_size) : ""; }

        //
        operator bool&() { return this->m_b; }
        operator bool const&() const { return this->m_b; }

        //
        operator uint8_t&() { return this->m_u8; }
        operator uint8_t const&() const { return this->m_u8; }

        //
        operator uint16_t&() { return this->m_u16; }
        operator uint16_t const&() const { return this->m_u16; }

        //
        operator uint32_t&() { return this->m_u32; }
        operator uint32_t const&() const { return this->m_u32; }

    private:
        tag_t m_tag = 0;
        union {
            long m_value;
            size_t m_size;
            uint32_t m_u32;
            uint16_t m_u16;
            uint8_t m_u8;
            bool m_b;
        };
        uint8_t const* m_payload;
        type_t m_type = 0;
    };
}

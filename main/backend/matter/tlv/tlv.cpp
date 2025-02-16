#include "./tlv.h"

//
#include <cstring>
#include <iomanip>

//
#include "limits"
#include "./std/hexdump.h"
#include "./std/stream_guard.h"

//
namespace tlvcpp
{
    static tag_parser g_tag_parser = nullptr;
    void set_tag_parser(tag_parser parser) { g_tag_parser = parser; }

    //
    bool tag_is_primitive(tag_t tag)
    {
        union { tag_t tag; uint8_t byte[sizeof(tag_t)]; } tag_bytes;
        tag_bytes.tag = tag;

        //
        for (size_t i = sizeof(tag_t) - 1; i != static_cast<std::size_t>(-1); i--)
            if (tag_bytes.byte[i]) return !(tag_bytes.byte[i] & 0b00100000);

        return false;
    }

    // from another
    tlv::tlv(const tlv &other) : 
        m_tag(other.m_tag), 
        m_value(other.m_value), 
        m_type(other.m_type), 
        m_payload(other.m_payload) 
        {};

    tlv::tlv(tlv &&other) noexcept
    {
        m_tag     = std::move(other.m_tag);
        m_value   = std::move(other.m_value);
        m_type    = std::move(other.m_type);
        m_payload = std::move(other.m_payload);
        other.m_tag = 0;
    }

    //
    tlv::~tlv() {}
    tlv &tlv::operator=(const tlv &other)
    {
        if (&other == this) return *this;
        m_type    = other.m_type;
        m_tag     = other.m_tag;
        m_value   = other.m_value;
        m_payload = other.m_payload;
        return *this;
    }

    //
    tlv &tlv::operator=(tlv &&other) noexcept
    {
        if (&other == this) return *this;
        m_tag     = std::move(other.m_tag);
        m_value   = std::move(other.m_value);
        m_payload = std::move(other.m_payload);
        m_type    = std::move(other.m_type);
        other.m_tag = 0;
        return *this;
    }

    //
    bool operator==(const tlv &tlv, const tag_t &tag) { return tlv.m_tag == tag; }

    //
    std::ostream &operator<<(std::ostream &stream, const tlv &tlv)
    {
        const auto width = static_cast<size_t>(stream.width());

        {   // needs C21 / C++23
            stream_guard<std::ostream> guard{stream};
            stream << "";
            if (g_tag_parser)
                { stream << "* tag: " << g_tag_parser(tlv.m_tag); } else
                { stream << "* tag: 0x" << std::hex << std::setw(2) << std::setfill('0') << tlv.m_tag; }
        }

        //hexdump(tlv.m_value, tlv.m_length, width + 4, stream);
        //if (tlv.m_length)
        //{ stream << ", length: " << tlv.m_length << ", value:\n"; } else { stream << '\n'; };

        //
        return stream;
    }

}
//hexdump(tlv.m_value, tlv.m_length, width + 4, stream);

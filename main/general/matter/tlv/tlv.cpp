#include "./tlv.h"
#include <cstring>

//
namespace tlvcpp {
    // from another
    tlv::tlv(tlv const& other) : m_control(other.m_control), m_tag(other.m_tag), m_value(other.m_value), m_payload(other.m_payload) {};
    tlv::tlv(tlv const& other, tag_t const& tag) : m_control(other.m_control), m_tag(tag), m_value(other.m_value), m_payload(other.m_payload) {};

    //
    tlv::tlv(tlv &&other) noexcept { m_tag = std::move(other.m_tag); m_value = std::move(other.m_value); m_control = std::move(other.m_control); m_payload = std::move(other.m_payload); other.m_tag = 0;}
    tlv::tlv(tlv &&other, tag_t const& tag) noexcept { m_tag = tag; m_value = std::move(other.m_value);m_control = std::move(other.m_control); m_payload = std::move(other.m_payload); other.m_tag = 0;}

    //
    tlv::~tlv() {}
    tlv &tlv::operator=(const tlv &other) { if (&other == this) return *this; m_control = other.m_control; m_tag = other.m_tag; m_value = other.m_value; m_payload = other.m_payload; return *this; }
    tlv &tlv::operator=(tlv &&other) noexcept { if (&other == this) return *this; m_tag = std::move(other.m_tag); m_value = std::move(other.m_value); m_payload = std::move(other.m_payload); m_control = std::move(other.m_control); other.m_tag = 0; return *this; }
}

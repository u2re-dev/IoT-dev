#include <assert.h>
#include <cstring>

//
#include "./tlv_tree.h"

//
namespace tlvcpp
{
    namespace TLV {
        namespace Constants {
            namespace Type {
                enum Type : uint8_t {
                    SIGNED_INTEGER        = 0x00,
                    UNSIGNED_INTEGER      = 0x04,
                    INT16                 = 0x05,
                    BOOLEAN               = 0x08,
                    FLOATING_POINT_NUMBER = 0x0A,
                    UTF8_STRING           = 0x0C,
                    BYTE_STRING           = 0x10,
                    NULL_TYPE             = 0x14,
                    STRUCTURE             = 0x15,
                    ARRAY                 = 0x16,
                    PATH                  = 0x17
                };
            }
        }
    }




    // ========== MATTER-SERIALIZATION CURRENTLY NOT SUPPORTED! =======

    //
    static length_t length_of_tag(const tag_t tag)
    {
        union { tag_t tag; uint8_t byte[sizeof(tag_t)]; } tag_bytes;
        tag_bytes.tag = tag;
        length_t length = 1;
        while (tag_bytes.byte[length]) {
            length++;
            if (length > sizeof(tag_t)) { length = sizeof(tag_t); break; }
        }
        return length;
    }

    //
    static length_t length_of_length(const length_t length)
    {
        length_t length_of_length = 1;
        if (length < 128) return length_of_length;
        union { length_t length; uint8_t byte[sizeof(length_t)]; } length_bytes;
        length_bytes.length = length;
        while (length_bytes.byte[length_of_length]) { length_of_length++; if (length_of_length > sizeof(length_t)) { length_of_length = sizeof(length_t); break; } }
        return length_of_length + 1;
    }

    //
    static length_t node_length_recursive(const tlv_tree_node &node)
    {
        /*
        const tlvcpp::tlv &tlv = node.data();
        length_t size = 0; size += length_of_tag(tlv.tag());

        //
        if (tag_is_primitive(tlv.tag())) size += tlv.length(); else
            for (const auto &child : node.children())
                size += node_length_recursive(child);

        size += length_of_length(size);
        return size;*/
        return 0;
    }

    //
    static length_t node_length(const tlv_tree_node &node)
    {
        /*
        const tlvcpp::tlv &tlv = node.data();
        if (tag_is_primitive(tlv.tag())) return tlv.length();

        length_t size = 0;
        for (const auto &child : node.children())
            size += node_length_recursive(child);

        return size;*/
        return 0;
    }

    //
    static bool serialize_tag(const tag_t tag, std::vector<uint8_t> &buffer)
    {
        if (!tag) return false;
        length_t length = length_of_tag(tag);

        union { tag_t tag; uint8_t byte[sizeof(tag_t)]; } tag_bytes;
        tag_bytes.tag = tag;

        for (size_t i = length - 1; i != static_cast<std::size_t>(-1); i--) {
            buffer.push_back(tag_bytes.byte[i]);
        }

        return true;
    }

    //
    static bool serialize_length(const length_t length, std::vector<uint8_t> &buffer)
    {
        if (length < 0b01111111) { buffer.push_back(static_cast<uint8_t>(length)); return true; }

        //
        union { length_t length; uint8_t byte[sizeof(length_t)]; } length_bytes;
        length_bytes.length = length;

        //
        uint8_t length_of_length = sizeof(length_t);
        for (size_t i = sizeof(length_t) - 1; i != static_cast<std::size_t>(-1); i--)
            if (!length_bytes.byte[i]) length_of_length--; else break;

        //
        if (length_of_length > sizeof(length_t)) return false;
        buffer.push_back(length_of_length | 0b10000000);

        //
        size_t byte_index = length_of_length - 1;
        while (byte_index != static_cast<std::size_t>(-1))
            buffer.push_back(length_bytes.byte[byte_index--]);

        return true;
    }

    static bool serialize_value(const uint8_t *value, const size_t size, std::vector<uint8_t> &buffer)
    {
        if (size)
            buffer.insert(buffer.end(), value, value + size);

        return true;
    }

    static bool serialize_recursive(const tlv_tree_node &node, std::vector<uint8_t> &buffer)
    {
        const tlvcpp::tlv &tlv = node.data();
        if (!serialize_tag(tlv.tag(), buffer)) return false;
        if (!serialize_length(node_length(node), buffer)) return false;

        //
        if (tag_is_primitive(tlv.tag()))
        {
            //if (!serialize_value(tlv.value(), tlv.length(), buffer))
                //return false;
        } else {
            for (const auto &child : node.children())
                if (!serialize_recursive(child, buffer)) return false;
        }

        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::serialize(std::vector<uint8_t> &buffer, size_t *bytes_written) const
    {
        const auto size = buffer.size();
        if (data().tag()) {
            if (!serialize_recursive(*this, buffer)) return false;
        } else {
            for (const auto &child : children())
                if (!serialize_recursive(child, buffer)) return false;
        }
        if (bytes_written) *bytes_written = buffer.size() - size;
        return true;
    }




    // ========== MATTER DESERIALIZATION =======

    static bool deserialize_tag( value_reader& reader, tlv& value )
    {
        if (!reader.checkMemory()) return false;

        //
        tag_t _tag = reader.readU8();
        if (!_tag) return false;

        // get simple-type of tag
        value.type(_tag & 0b00011111);

        // is really simple type i.e. classic TLV?
        if ((_tag ^ value.type()) <= 0b00011111) { value.tag(value.type()); return (value.type() == 0x18) ? false : true; }

        // matter tags always complex...
        value.tag(reader.readU8());
        if (!reader.checkMemory()) return false;

        //
        switch (value.type())
        {
            // parsing structure END-TAG
            case 0x18: return false;

            // due of 0x15 may be primitive tag...
            case TLV::Constants::Type::STRUCTURE: return true;
            case TLV::Constants::Type::UTF8_STRING:

            //
            case TLV::Constants::Type::ARRAY:
            case TLV::Constants::Type::PATH:
            case TLV::Constants::Type::BYTE_STRING: {
                //
                uint8_t next_byte = reader.readU8();
                length_t add_len  = 0; if (!(next_byte & 0b10000000)) add_len = next_byte; // simple length value

                // long length value
                while ((next_byte & 0b10000000) && reader.checkMemory())
                {   // reconstruct length
                    add_len   = (add_len << 7) | (next_byte & 0b01111111);
                    next_byte = reader.readU8();
                }
                if (!reader.checkMemory()) return false;

                //
                value.setBytes((uint8_t const*)reader.readBytes(add_len), add_len);
                return true;
            }; 

            case TLV::Constants::Type::UNSIGNED_INTEGER:
                value = reader.readU8();
                return true;

            case TLV::Constants::Type::INT16:
                reader.checkMemory(2);
                value = reader.readU16();
                return true;

            case TLV::Constants::Type::SIGNED_INTEGER:
            case TLV::Constants::Type::FLOATING_POINT_NUMBER:
                reader.checkMemory(4);
                value = reader.readU16();
                return true;

            case TLV::Constants::Type::BOOLEAN:
                value = true; // TODO: set value as true
                return true;

            case TLV::Constants::Type::NULL_TYPE:
                return true;

            default: // may be classic TLV (may be broken or currupted)
                while (reader.checkMemory() && (_tag & 0b10000000)) {
                    _tag = (_tag << 8) | reader.readU8();
                };
                _tag &= 0b00011111; value.tag(_tag);
                return reader.checkMemory();
        }
        return false;
    }

    //
    static bool deserialize_recursive(value_reader& reader, tlv_tree_node &node, intptr_t level = 0)
    {
        while (reader.checkMemory())
        {
            tlv value{ 0 };
            if (!deserialize_tag(reader, value)) return false;
            if (!reader.checkMemory()) return false;

            //! unfixable issue: type 0x15 should be `node` itself!
            //! if use such trick, will no visible 0x15 tag-name at all
            auto &child = (value.type() == 0x15 && level == 0) ? node : node.add_child(value);
            if (!(value.type() == 0x15 ? deserialize_recursive(reader, child, level+1) : true)) return false;
        }

        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(value_reader& reader)
    {
        if (!deserialize_recursive(reader, *this)) return false;

        if (this->data().tag() == 0 && children().size() == 1)
            *this = std::move(children().front());

        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(uint8_t const* data, size_t size) {
        auto reader = value_reader(data, size);
        return deserialize(reader);
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(std::vector<uint8_t> const& buffer){
        return deserialize(buffer.data(), buffer.size());
    }
}

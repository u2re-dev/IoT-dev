#include <assert.h>
#include <cstring>

#include "./tlv_tree.h"

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
        union
        {
            tag_t tag;
            uint8_t byte[sizeof(tag_t)];
        } tag_bytes;

        tag_bytes.tag = tag;

        length_t length = 1;

        while (tag_bytes.byte[length])
        {
            length++;

            if (length > sizeof(tag_t))
            {
                length = sizeof(tag_t);

                break;
            }
        }

        return length;
    }

    static length_t length_of_length(const length_t length)
    {
        length_t length_of_length = 1;

        if (length < 128)
            return length_of_length;

        union
        {
            length_t length;
            uint8_t byte[sizeof(length_t)];
        } length_bytes;

        length_bytes.length = length;

        while (length_bytes.byte[length_of_length])
        {
            length_of_length++;

            if (length_of_length > sizeof(length_t))
            {
                length_of_length = sizeof(length_t);

                break;
            }
        }

        return length_of_length + 1;
    }

    static length_t node_length_recursive(const tlv_tree_node &node)
    {
        const tlvcpp::tlv &tlv = node.data();
        length_t size = 0;

        size += length_of_tag(tlv.tag());

        if (tag_is_primitive(tlv.tag()))
            size += tlv.length();
        else
            for (const auto &child : node.children())
                size += node_length_recursive(child);

        size += length_of_length(size);

        return size;
    }

    static length_t node_length(const tlv_tree_node &node)
    {
        const tlvcpp::tlv &tlv = node.data();

        if (tag_is_primitive(tlv.tag()))
            return tlv.length();

        length_t size = 0;

        for (const auto &child : node.children())
            size += node_length_recursive(child);

        return size;
    }

    static bool serialize_tag(const tag_t tag, std::vector<uint8_t> &buffer)
    {
        if (!tag)
            return false;

        length_t length = length_of_tag(tag);

        union
        {
            tag_t tag;
            uint8_t byte[sizeof(tag_t)];
        } tag_bytes;

        tag_bytes.tag = tag;

        for (size_t i = length - 1; i != static_cast<std::size_t>(-1); i--)
            buffer.push_back(tag_bytes.byte[i]);

        return true;
    }

    static bool serialize_length(const length_t length, std::vector<uint8_t> &buffer)
    {
        if (length < 0b01111111)
        {
            buffer.push_back(static_cast<uint8_t>(length));

            return true;
        }

        union
        {
            length_t length;
            uint8_t byte[sizeof(length_t)];
        } length_bytes;

        length_bytes.length = length;

        uint8_t length_of_length = sizeof(length_t);

        for (size_t i = sizeof(length_t) - 1; i != static_cast<std::size_t>(-1); i--)
            if (!length_bytes.byte[i])
                length_of_length--;
            else
                break;

        if (length_of_length > sizeof(length_t))
            return false;

        buffer.push_back(length_of_length | 0b10000000);

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

        if (!serialize_tag(tlv.tag(), buffer))
            return false;

        if (!serialize_length(node_length(node), buffer))
            return false;

        if (tag_is_primitive(tlv.tag()))
        {
            if (!serialize_value(tlv.value(), tlv.length(), buffer))
                return false;
        }
        else
            for (const auto &child : node.children())
                if (!serialize_recursive(child, buffer))
                    return false;

        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::serialize(std::vector<uint8_t> &buffer, size_t *bytes_written) const
    {
        const auto size = buffer.size();

        if (data().tag())
        {
            if (!serialize_recursive(*this, buffer))
                return false;
        }
        else
            for (const auto &child : children())
                if (!serialize_recursive(child, buffer))
                    return false;

        if (bytes_written)
            *bytes_written = buffer.size() - size;

        return true;
    }




    // ========== MATTER DESERIALIZATION =======

    //
    static bool deserialize_tag(const uint8_t *&buffer, size_t &remaining_size, 
          type_t &type,
           tag_t &tag,
        length_t &length, 
        uint8_t *& value
    )
    {
        if (!remaining_size) return false;

        //
        tag_t _tag = *buffer++; remaining_size--; length++;
        if (!_tag) return false;

        // get simple-type of tag
        type = _tag & 0b00011111;

        // is really simple type i.e. classic TLV?
        if ((_tag ^ type) <= 0b00011111)
        {   //
            tag = type;
            //buffer++; remaining_size--; length++; // needs this line or not?
            if (type == 0x18) { return false; }; // means parse end
            value = nullptr; return true;
        }

        // matter tags always complex...
        tag = *buffer++; remaining_size--; // use seq. number as tag-name
        if (!remaining_size) return false;

        //
        switch (type)
        {
            // parsing structure END-TAG
            case 0x18: return false;

            // due of 0x15 may be primitive tag...
            case TLV::Constants::Type::STRUCTURE:
                //length++; buffer++; remaining_size--; // needs this line or not?
                value = nullptr; return true;

            //
            case TLV::Constants::Type::UTF8_STRING:

            //
            case TLV::Constants::Type::ARRAY:
            case TLV::Constants::Type::PATH:
            case TLV::Constants::Type::BYTE_STRING: {
                //
                uint8_t next_byte = *buffer++; remaining_size --;
                length_t add_len  = 0;
                if (!(next_byte & 0b10000000)) add_len = next_byte; // simple length value

                // long length value
                while ((next_byte & 0b10000000) && remaining_size > 0)
                {   // reconstruct length
                    add_len   = (add_len << 7) | (next_byte & 0b01111111);
                    next_byte = *buffer++; remaining_size --; length++;
                }
                if (remaining_size == 0) return false;

                //
                value = (uint8_t*)(buffer); // TODO: payload length i.e. `add_len`
                length += add_len; buffer += add_len; remaining_size -= add_len; // just skip those bytes
                return true;
            }; 

            case TLV::Constants::Type::UNSIGNED_INTEGER:
                if (remaining_size < 1) return false;
                value = (uint8_t*)(buffer);
                length += 1; buffer += 1; remaining_size -= 1;
                return true;

            case TLV::Constants::Type::INT16:
                if (remaining_size < 2) return false;
                value = (uint8_t*)(buffer);
                length += 2; buffer += 2; remaining_size -= 2;
                return true;

            case TLV::Constants::Type::SIGNED_INTEGER:
            case TLV::Constants::Type::FLOATING_POINT_NUMBER:
                if (remaining_size < 4) return false;
                value = (uint8_t*)(buffer);
                length += 4; buffer += 4; remaining_size -= 4;
                return true;

            case TLV::Constants::Type::BOOLEAN:
                value = nullptr; // TODO: set value as true
                return true;

            case TLV::Constants::Type::NULL_TYPE:
                value = nullptr;
                return true;

            default: // may be classic TLV (may be broken or currupted)
                while (remaining_size > 0 && (_tag & 0b10000000)) {
                    _tag = (_tag << 8) | (*buffer++); remaining_size--; length++;
                };
                _tag &= 0b00011111; tag = _tag; if (remaining_size == 0) return false;
        }
        return false;
    }

    //
    static bool deserialize_recursive(const uint8_t *&buffer, size_t &remaining_size, tlv_tree_node &node)
    {
        while (remaining_size)
        {
            tag_t tag = 0; length_t length = 0; type_t type = 0;
            uint8_t* value = nullptr;

            //
            if (!deserialize_tag(buffer, remaining_size, type, tag, length, value)) return false;
            if (remaining_size == 0) return false;

            //! unfixable issue: type 0x15 should be `node` itself!
            //! if use such trick, will no visible 0x15 tag-name at all
            auto &child = type == 0x15 ? node : (node.add_child(tag, length, value));
            if (!(type == 0x15 ? deserialize_recursive(buffer, remaining_size, child) : true)) return false;
        }

        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(const uint8_t *buffer, const size_t size)
    {
        size_t remaining_size = size;

        if (!deserialize_recursive(buffer, remaining_size, *this))
            return false;

        if (this->data().tag() == 0 && children().size() == 1)
            *this = std::move(children().front());

        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(const std::vector<uint8_t> &buffer)
    {
        return deserialize(buffer.data(), buffer.size());
    }
}

#ifndef F209A449_A8CD_4B1F_BEDD_F1DA13D149BD
#define F209A449_A8CD_4B1F_BEDD_F1DA13D149BD

//
#include "./tlv.h"
#include "./enums.hpp"
//#include "./std/tree.h"
//#include "./serialize.hpp"
#include "./tlv_tree.h"

//
namespace tlvcpp
{
    // ========== MATTER DESERIALIZATION ========
    static bool deserialize_tag( data_reader& reader, tlv& value )
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

            //? may have different writing style (instead of length, may have zero ending before next tag)
            case TLV::Constants::Type::UTF8_STRING:

            //? may have different writing style (have additional type identifier)
            case TLV::Constants::Type::ARRAY:

            //
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

            //
            case TLV::Constants::Type::UNSIGNED_INTEGER:
                value = reader.readU8();
                return true;

            //
            case TLV::Constants::Type::INT16:
                reader.checkMemory(2);
                value = reader.readU16();
                return true;

            //
            case TLV::Constants::Type::SIGNED_INTEGER:
            case TLV::Constants::Type::FLOATING_POINT_NUMBER: // TODO: really support floating point
                reader.checkMemory(4);
                value = reader.readU16();
                return true;

            // booleans in general has flag to next optional tag in structure (i.e. mark)
            case TLV::Constants::Type::BOOLEAN: value = true; return true;
            case TLV::Constants::Type::NULL_TYPE: return true;

            //
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
    static bool deserialize_recursive(data_reader& reader, tlv_tree_node &node, intptr_t level = 0)
    {
        while (reader.checkMemory())
        {
            tlv value{ 0 };
            if (!deserialize_tag(reader, value)) return false;
            if (!reader.checkMemory()) return false;

            //
            bool isStructure = value.type() == 0x15;
            auto& child = (isStructure && level == 0) ? node : node.add_child(value);
            if (isStructure) node.data() = value; // fix tag type issue
            if (!(isStructure ? deserialize_recursive(reader, child, level+1) : true)) return false;
        }
        return true;
    }
}

#endif /* F209A449_A8CD_4B1F_BEDD_F1DA13D149BD */

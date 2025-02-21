#ifndef E5F5B917_5190_430A_98EA_FD71CC83AB96
#define E5F5B917_5190_430A_98EA_FD71CC83AB96

//
//#include "./std/tree.h"
#include "./enums.hpp"
#include "../tlv.h"
#include "../tlv_tree.h"

//
#include "../../std/types.hpp"



// ========== MATTER-SERIALIZATION! =======
// TODO: partialy compatibility with classic TLV
namespace tlvcpp
{

    //
    inline bool writeOctets(writer_t& writer, tlv const& value, uint8_t const& type) {
        uint8_t octet = type&0b00000011;
        switch (octet)
        {
            case 0: writer.writeByte(value); break;
            case 1: writer.writeUInt16(value); break;
            case 2: writer.writeUInt32(value); break;
            case 3: writer.writeUInt64(value); break;
        }
        return true;
    }

    //
    inline bool serialize_recursive(const tlv_tree_node& node, writer_t& writer, uintptr_t level = 0)
    {
        auto& element = node.data();
        type_t type = element.type();
        tag_t tag   = element.tag();

        // also, don't write zero tags
        if (type && type != e_type::END) { // don't write anothing about such tag
            bool isSimple = (level == 0); // TODO: better complex type definition
            writer.writeByte(!isSimple ? (type | 0b00100000) : type);
            if (!isSimple) { writer.writeByte(tag); }
        };

        //
        switch (type & 0b00011100)
        {
            case e_type::END: return false; // skip that tag...
            case e_type::STRUCTURE:
                for (const auto& child : node.children()) {
                    if (!serialize_recursive(child, writer, level + 1)) {
                        writer.writeByte(0x18); return true;
                    }
                }
                writer.writeByte(0x18);
                return true;

            case e_type::BYTE_STRING:
            case e_type::UTF8_STRING:
                writer.writeByte(element.size());
                writer.writeBytes(element.payload(), element.size());
                return true;

            case e_type::SIGNED_INTEGER:
            case e_type::UNSIGNED_INTEGER:
                writeOctets(writer, element, element.type());
                return true;

            case e_type::FLOATING_POINT_NUMBER:
                writer.writeUInt32(static_cast<uint32_t>(element));
                return true;

            default:
                return true;
        }
        return false;
    }
}

#endif /* E5F5B917_5190_430A_98EA_FD71CC83AB96 */

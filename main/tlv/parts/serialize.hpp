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
        control_t control = element.control();

        // also, don't write zero tags
        if (control.type && control.type != e_type::END) { // don't write anothing about such tag
            bool isSimple = control.lab == 0; // TODO: better complex type definition
            writer.writeByte(reinterpret_cast<uint8_t&>(control));
            if (!isSimple) { writer.writeByte(element.tag()); };
        };

        //
        switch (control.type)
        {
            case e_type::END: return false; // skip that tag...
            case e_type::STRUCTURE:
                for (const auto& child : node.children()) {
                    if (!serialize_recursive(child, writer, level + 1)) { break; }
                }
                writer.writeByte(0x18); 
                return true;

            //
            case e_type::UTF8_STRING:
            case e_type::SIGNED_INTEGER:
            case e_type::UNSIGNED_INTEGER:
            case e_type::FLOATING_POINT:
            case e_type::BYTE_STRING:
                if (!writeOctets(writer, element, control.octet)) return false;
                if ( element.payload() && (control.type == e_type::BYTE_STRING || control.type == e_type::UTF8_STRING)) 
                    { writer.writeBytes(element.payload(), element.size()); };
                return true;

            default: return true;
        }
        return false;
    }
}

#endif /* E5F5B917_5190_430A_98EA_FD71CC83AB96 */

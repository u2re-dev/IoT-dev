#ifndef E5F5B917_5190_430A_98EA_FD71CC83AB96
#define E5F5B917_5190_430A_98EA_FD71CC83AB96

//
//#include "./std/tree.h"
#include "./enums.hpp"
#include "./tlv.h"
#include "./tlv_tree.h"

//
namespace tlvcpp
{
    // ========== MATTER-SERIALIZATION! =======
    // TODO: partialy compatibility with classic TLV
    static bool serialize_recursive(const tlv_tree_node &node, data_writer& writer, intptr_t level = 0) {
        auto& tlv = node.data();
        auto type      = tlv.type();
        auto tag        = tlv.tag();

        // type also tagname of classic TLV, also from level-1 detaultly complex-type
        bool isSimple = level == 0;
        writer.writeByte(!isSimple ? (type | 0b00100000) : type); // TODO: write correct tag (primitive or complex i.e. with mask 0b11100000)
        if (!isSimple) { writer.writeByte(tag); }; // TODO: correctly write primitives

        //
        //! no childrens writing, except 0x15, won't support

        // also, any structs 
        switch (type) {
            case 0x15: // currently, all tags from level=1 defaultly complex-type
                for (const auto &child : node.children())
                    if (!serialize_recursive(child, writer, level + 1)) { 
                        writer.writeByte(0x18); return false;
                    };
                writer.writeByte(0x18); // end-with
            return true;

            // not viable tag (end writing level)
            case 0x18: return false;

            //? may have different writing style (have additional type identifier)
            case TLV::Constants::Type::ARRAY:

            //? may have different writing style (instead of length, may have zero ending before next tag)
            case TLV::Constants::Type::UTF8_STRING:

            //
            case TLV::Constants::Type::PATH:
            case TLV::Constants::Type::BYTE_STRING:
             // TODO: support complex length types (with 0b10000000 mask overflow)
            writer.writeByte(tlv.size()); writer.writeBytes(tlv.payload(), tlv.size()); return true;

            //
            case TLV::Constants::Type::UNSIGNED_INTEGER: writer.writeByte((uint8_t)tlv); return true;
            case TLV::Constants::Type::INT16:            writer.writeShort((uint16_t)tlv); return true;

            //
            case TLV::Constants::Type::SIGNED_INTEGER:
            case TLV::Constants::Type::FLOATING_POINT_NUMBER: // TODO: really support floating point
                writer.writeInt((uint32_t)tlv); return true;

            // skip-writing next...
            case TLV::Constants::Type::BOOLEAN  : return true;
            case TLV::Constants::Type::NULL_TYPE: return true;

            //
            default: return true; // currently, unknown tags won't supported
        }
        return false;
    }
}

#endif /* E5F5B917_5190_430A_98EA_FD71CC83AB96 */

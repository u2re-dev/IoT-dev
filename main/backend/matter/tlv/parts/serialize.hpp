#ifndef E5F5B917_5190_430A_98EA_FD71CC83AB96
#define E5F5B917_5190_430A_98EA_FD71CC83AB96

//
//#include "./std/tree.h"
#include "./data.hpp"
#include "./enums.hpp"
#include "../tlv.h"
#include "../tlv_tree.h"

//
namespace tlvcpp
{
    // ========== MATTER-SERIALIZATION! =======
    // TODO: partialy compatibility with classic TLV

    static bool serialize_recursive(const tlv_tree_node& node, data_writer& writer, intptr_t level = 0)
    {
        auto& element = node.data();
        type_t type = element.type();
        tag_t tag = element.tag();

        bool isSimple = (level == 0);
        writer.writeByte(!isSimple ? (type | 0b00100000) : type);
        if (!isSimple) { writer.writeByte(tag); }

        switch (type)
        {
            case e_type::STRUCTURE:
                // Рекурсивная сериализация дочерних элементов, завершающая тегом 0x18
                for (const auto& child : node.children()) {
                    if (!serialize_recursive(child, writer, level + 1)) {
                        writer.writeByte(0x18);
                        return false;
                    }
                }
                writer.writeByte(0x18);
                return true;

            case e_type::BYTE_STRING:
            case e_type::PATH:
            case e_type::UTF8_STRING:
                writer.writeByte(element.size());
                writer.writeBytes(element.payload(), element.size());
                return true;

            case e_type::UNSIGNED_INTEGER:
                writer.writeByte(static_cast<uint8_t>(element));
                return true;

            case e_type::INT16:
                writer.writeShort(static_cast<uint16_t>(element));
                return true;

            case e_type::SIGNED_INTEGER:
            case e_type::FLOATING_POINT_NUMBER:
                writer.writeInt(static_cast<uint32_t>(element));
                return true;

            case e_type::BOOLEAN:
            case e_type::NULL_TYPE:
                return true;

            default:
                return true;
        }
        return false;
    }
}

#endif /* E5F5B917_5190_430A_98EA_FD71CC83AB96 */

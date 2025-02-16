#ifndef F209A449_A8CD_4B1F_BEDD_F1DA13D149BD
#define F209A449_A8CD_4B1F_BEDD_F1DA13D149BD

//
#include "./data.hpp"
#include "./enums.hpp"
#include "../tlv.h"
#include "../tlv_tree.h"

//
namespace tlvcpp
{
    // ========== MATTER DESERIALIZATION ========
    static bool deserialize_tag(data_reader& reader, tlv& value)
    {
        if (!reader.checkMemory()) return false;
        tag_t rawTag = reader.readU8();
        if (!rawTag) return false;

        //
        value.type(rawTag & 0b00011111);

        //
        if ((rawTag ^ value.type()) <= 0b00011111) {
            value.tag(value.type());
            return (value.type() != 0x18);
        }

        //
        value.tag(reader.readU8());
        if (!reader.checkMemory()) return false;

        //
        switch (value.type())
        {
            case 0x18: return false;
            case e_type::STRUCTURE: return true;
            case e_type::BYTE_STRING:
            case e_type::PATH:
            case e_type::UTF8_STRING:
            {
                uint8_t next_byte = reader.readU8();
                length_t length = 0;
                // Если значение длины в одном байте – без маски
                if (!(next_byte & 0b10000000)) {
                    length = next_byte;
                } else {
                    // Длинное значение длины (с маской overflow)
                    while (next_byte & 0b10000000 && reader.checkMemory())
                    {
                        length = (length << 7) | (next_byte & 0b01111111);
                        next_byte = reader.readU8();
                    }
                }
                if (!reader.checkMemory()) return false;
                value.setBytes(reinterpret_cast<const uint8_t*>(reader.readBytes(length)), length);
                return true;
            }

            case e_type::UNSIGNED_INTEGER:
                value = reader.readU8();
                return true;

            case e_type::INT16:
                if (!reader.checkMemory(2)) return false;
                value = reader.readU16();
                return true;

            case e_type::SIGNED_INTEGER:
            case e_type::FLOATING_POINT_NUMBER:
                if (!reader.checkMemory(4)) return false;
                value = reader.readU16(); // В оригинальном коде читается U16; при необходимости заменить на readU32()
                return true;

            case e_type::BOOLEAN:
                value = true;
                return true;

            case e_type::NULL_TYPE:
                return true;

            default:
                // Обработка классического TLV (сетевой порядок или прочее)
                while (reader.checkMemory() && (rawTag & 0b10000000)) {
                    rawTag = (rawTag << 8) | reader.readU8();
                }
                value.tag(rawTag & 0b00011111);
                return reader.checkMemory();
        }
        return false;
    }

    //
    static bool deserialize_recursive(data_reader& reader, tlv_tree_node& node, intptr_t level = 0)
    {
        while (reader.checkMemory())
        {
            tlv value{ 0 };
            if (!deserialize_tag(reader, value)) return false;
            if (!reader.checkMemory()) return false;

            //
            bool isStructure = (value.type() == e_type::STRUCTURE);
            auto& child = (isStructure && level == 0) ? node : node.add_child(value);
            if (isStructure) node.data() = value;
            if (isStructure && !deserialize_recursive(reader, child, level + 1)) return false;
        }
        return true;
    }
}

#endif

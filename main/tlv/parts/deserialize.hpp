#ifndef F209A449_A8CD_4B1F_BEDD_F1DA13D149BD
#define F209A449_A8CD_4B1F_BEDD_F1DA13D149BD

//
#include "./enums.hpp"
#include "../tlv.h"
#include "../tlv_tree.h"



//
namespace tlvcpp
{
    //
    inline uint8_t width_from_bits(uint8_t bits) {
        switch (bits & 0x03) {
            case 0x00: return 1;
            case 0x01: return 2;
            case 0x02: return 4;
            case 0x03: return 8;
        }
        return 1;
    }

    //
    inline bool readOctets(reader_t& reader, auto& value, uint8_t const& type) {
        uint8_t octet = type&0b00000011;
        switch (octet) {
            case 0: if (!reader.checkMemory(1)) return false; value = reader.readByte(); break;
            case 1: if (!reader.checkMemory(2)) return false; value = reader.readUInt16(); break;
            case 2: if (!reader.checkMemory(4)) return false; value = reader.readUInt32(); break;
            case 3: if (!reader.checkMemory(8)) return false; value = reader.readUInt64(); break;
        }
        return true;
    }

    // ========== MATTER DESERIALIZATION ========
    static bool deserialize_tag(reader_t& reader, tlv& value)
    {
        if (!reader.checkMemory()) return false;
        control_t control = reinterpret_cast<control_t&>(reader.readByte());

        //
        value = uint64_t(0);
        value.control(control);
        value.tag(control.lab == 0 ? 0 : reader.readByte());
        if (!reader.checkMemory()) return false;

        //
        switch (control.type)
        {
            case e_type::END: return false;
            case e_type::STRUCTURE: return true; // TODO: support of subtypes

            //
            case e_type::FLOATING_POINT: // if boolean, just return true or false
                if ((control.octet&0b10) == 0) { value = control.octet != 0; return true; }

            // octet-typed
            case e_type::UTF8_STRING:
            case e_type::BYTE_STRING:
            case e_type::SIGNED_INTEGER:
            case e_type::UNSIGNED_INTEGER:
                if (!readOctets(reader, value, control.octet)) return false;
                if (control.type == e_type::BYTE_STRING) {
                    value.setBytes(reinterpret_cast<const uint8_t*>(reader.allocate(value)), value);
                }; return true; break;

            //
            default: return reader.checkMemory();
        }
        return false;
    }

    //
    static bool deserialize_recursive(reader_t& reader, tlv_tree_node& node, intptr_t level = 0) {
        while (reader.checkMemory()) {
            tlv value{};
            if (!deserialize_tag(reader, value)) return false;
            if (!reader.checkMemory()) return false;
            bool isStruct = value.type() == e_type::STRUCTURE;
            auto& child = (isStruct && level == 0) ? node : node.add_child(value);
            if (isStruct) {
                child.data() = value; // assign type to node itself
                if (!deserialize_recursive(reader, child, level + 1)) return false;
            }
        }
        return true;
    }
}

#endif

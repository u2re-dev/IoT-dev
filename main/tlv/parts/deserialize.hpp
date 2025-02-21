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


    /* // unused
    uint8_t next_byte = reader.readByte();
    length_t length = 0;

    //
    if (!(next_byte & 0b10000000)) {
        length = next_byte;
    } else {
        while (next_byte & 0b10000000 && reader.checkMemory())
        { length = (length << 7) | (next_byte & 0b01111111); next_byte = reader.readByte(); }
    }*/


    // ========== MATTER DESERIALIZATION ========
    static bool deserialize_tag(reader_t& reader, tlv& value)
    {
        if (!reader.checkMemory()) return false;
        tag_t rawTag = reader.readByte();

        //
        value.type(rawTag & 0b00011111);
        bool isSimple = !(rawTag & 0b11100000);//(rawTag ^ value.type()) <= 0b00011111;

        // TODO: better definition
        value.tag(isSimple ? 0 : reader.readByte());
        if (!reader.checkMemory()) return false;

        //
        switch (value.type() & 0b00011100)
        {
            case e_type::END: return false;

            // TODO: support of 0x15 i.e. "has", 0x16 i.e. "array" and 0x17 i.e. "path"
            case e_type::STRUCTURE: return true; 

            //
            case e_type::UTF8_STRING:
            case e_type::BYTE_STRING: {
                uint64_t length = 0; if (!readOctets(reader, length, value.type())) return false;
                value.setBytes(reinterpret_cast<const uint8_t*>(reader.allocate(length)), length);
                return true;
            } break;

            case e_type::SIGNED_INTEGER:
            case e_type::UNSIGNED_INTEGER:
                return readOctets(reader, value, value.type());

            case e_type::FLOATING_POINT_NUMBER:
                if (!reader.checkMemory(4)) return false;
                value = reader.readUInt32(); //value.type()
                return true;

            case e_type::BOOLEAN:
                value = (value.type() & 0b00000001) == 1;
                return true;

            default:
                while (reader.checkMemory() && (rawTag & 0b10000000)) {
                    rawTag = (rawTag << 8) | reader.readByte();
                }
                value.tag(rawTag & 0b00011111);
                return reader.checkMemory();
        }
        return false;
    }

    //
    static bool deserialize_recursive(reader_t& reader, tlv_tree_node& node, intptr_t level = 0) {
        while (reader.checkMemory()) {
            tlv value{ 0 };
            if (!deserialize_tag(reader, value)) return false;
            if (!reader.checkMemory()) return false;
            bool isStruct = (value.type() & 0b00011100) == e_type::STRUCTURE;
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

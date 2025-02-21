#ifndef F209A449_A8CD_4B1F_BEDD_F1DA13D149BD
#define F209A449_A8CD_4B1F_BEDD_F1DA13D149BD

//
#include "./utils.hpp"

//
namespace tlvcpp
{


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
        switch (control.type) {
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

            // In case if in structure is one element (but we doesn't know how many)
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

#ifndef DBDB5449_91AD_407E_A3CF_C2769A3D3E70
#define DBDB5449_91AD_407E_A3CF_C2769A3D3E70

//
#include "./utils.hpp"
#include "../tlv_tree.h"
#include "../tlv.h"

//
namespace tlvcpp {
    //
    inline std::string debugOctets(tlv const& value, uint8_t const& type,  uint8_t const& base = 10) {
        uint8_t octet = type&0b00000011;
        switch (octet) { // TODO: support of more types
            case 0: return std::to_string(uint8_t(value)); break;
            case 1: return std::to_string(uint16_t(value)); break;
            case 2: return std::to_string(uint32_t(value)); break;
            case 3: return std::to_string(uint64_t(value)); break;
        }
        return "";
    }

    //
    inline std::string indent(uintptr_t level) {
        return std::string(level * 2, ' ');
    }

    //
    inline std::string controlInfo(control_t const& control) {
        std::ostringstream oss;
        oss << "type=" << std::to_string(control.type)
            << ", lab=" << std::to_string(control.lab)
            << ", octet=0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(control.octet) << std::dec;
        return oss.str();
    }

    //
    inline void debug_print_recursive(tlv_tree_node const& node, uintptr_t level = 0) {
        auto& element = node.data(); control_t control = element.control();

        //
        std::cout << "[DEBUG] " << indent(level) << "Node: Control{" << controlInfo(control) << "}, Tag=" << static_cast<int>(element.tag()) << ", PayloadPtr=" << std::hex << static_cast<const void*>(element.payload()) << std::endl;

        //
        if (control.type == e_type::END) {
            std::cout << "[DEBUG] " << indent(level) << "END tag encountered. Skipping further processing of this branch." << std::endl; return;
        }

        //
        if (control.type == e_type::STRUCTURE) {
            std::cout << "[DEBUG] " << indent(level) << "STRUCTURE (Count=" << node.children().size() << ") with Nodes" << std::endl;
            std::cout << "[DEBUG] " << indent(level) << "{{" << std::endl;
            for (const auto& child : node.children()) {
                debug_print_recursive(child, level + 1);
            }
            std::cout << "[DEBUG] " << indent(level) << "}}" << std::endl; return;
        }

        //
        if (control.type == e_type::UTF8_STRING) {
            std::cout << "[DEBUG] " << indent(level+1) << "UTF8_STRING: ";
            if (element.payload()) {
                std::cout << "\"" << std::string(element.payload(), element.payload() + element.size()) << "\"" << std::endl;
            } else {
                std::cout << "<None>" << std::endl;
            }
            return;
        }

        //
        if (control.type == e_type::SIGNED_INTEGER ||
            control.type == e_type::UNSIGNED_INTEGER ||
            control.type == e_type::FLOATING_POINT ||
            control.type == e_type::BYTE_STRING)
        {
            std::cout << "[DEBUG] " << indent(level+1)
                << "Data Type (" << static_cast<int>(control.type)
                << "). Octet: 0x" << std::hex << static_cast<int>(control.octet)
                << std::dec << std::endl;

            //
            if (element.payload()) {
                const uint8_t* p = reinterpret_cast<const uint8_t*>(element.payload());
                std::cout << "[DEBUG] " << indent(level+1) << "Payload Size: " << std::to_string(element.size()) << std::endl;
                std::cout << "[DEBUG] " << indent(level+1) << "Payload Data: 0x" << hex::b2h(bytespan_t(element.payload(), element.size())) << std::endl;
            } else {
                std::cout << "[DEBUG] " << indent(level+1) << "Octet Value: " << debugOctets(element, control.octet) << std::endl;
            }
            return;
        }

        //
        std::cout  << "[DEBUG] " << indent(level) << "Node of Type " << static_cast<int>(control.type) << " processed." << std::endl;
    }
}

#endif /* DBDB5449_91AD_407E_A3CF_C2769A3D3E70 */

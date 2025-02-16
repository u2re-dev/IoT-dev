#include <assert.h>
#include <cstring>

//
#include "./tlv.h"
#include "./std/tree.h"
#include "./serialize.hpp"
#include "./deserialize.hpp"

//
namespace tlvcpp
{
    //
    template<> 
    bool tree_node<tlvcpp::tlv>::serialize(data_writer& writer) const
    {
        if (data().type() == 0x15) { // any structure always begins byself
            if (!serialize_recursive(*this, writer)) return false;
        } else { // any other is child-written
            for (const auto &child : children())
                if (!serialize_recursive(child, writer)) return false;
        }
        return true;
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(data_reader& reader)
    {
        if (!deserialize_recursive(reader, *this)) return false;
        if (this->data().tag() == 0 && children().size() == 1)
            *this = std::move(children().front());
        return true;
    }



    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(uint8_t const* data, size_t size) {
        auto reader = data_reader(data, size);
        return deserialize(reader);
    }

    template <>
    bool tree_node<tlvcpp::tlv>::deserialize(std::vector<uint8_t> const& buffer){
        return deserialize(buffer.data(), buffer.size());
    }
}

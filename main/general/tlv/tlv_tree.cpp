//
#include <assert.h>
#include <cstring>

//
#include "./parts/serialize.hpp"
#include "./parts/deserialize.hpp"

//
namespace tlvcpp { //
    template<> bool tree_node<tlvcpp::tlv>::serialize(writer_t& writer) const {
        // any structure always begins byself
        if (data().type() == 0b101)
            { if (!serialize_recursive(*this, writer)) return false; } else
            { for (const auto &child : children())  // any other is child-written
                if (!serialize_recursive(child, writer)) return false; }
        return true;
    }

    //
    template <> bool tree_node<tlvcpp::tlv>::deserialize(reader_t& reader) {
        if (!deserialize_recursive(reader, *this)) return false;
        if (this->data().tag() == 0 && children().size() == 1) *this = std::move(children().front());
        return true;
    }

    //
    template <> bool tree_node<tlvcpp::tlv>::deserialize(uint8_t const* data, size_t const& size) { auto reader = reader_t(data, size); return deserialize(reader); }
    template <> bool tree_node<tlvcpp::tlv>::deserialize(std::vector<uint8_t> const& buffer) { return deserialize(buffer.data(), buffer.size()); }
    template <> bool tree_node<tlvcpp::tlv>::deserialize(  std::span<uint8_t> const& buffer) { return deserialize(buffer.data(), buffer.size()); }
    template <> bool tree_node<tlvcpp::tlv>::deserialize(bytespan_t const& buffer) { return deserialize(buffer->data(), buffer->size()); }
    template <> bool tree_node<tlvcpp::tlv>::deserialize(bytes_t const& buffer) { return deserialize(buffer->data(), buffer->size()); }
};

#include "../libtuya.hpp"

//
namespace tc {
    std::array<uint32_t, 2> prepareJSON(uint8_t*data, size_t& length, char const* protocolVersion, uint8_t* output) {
        // protocol 3.4 - encrypted with header
        const auto encLen = ((length + 15 + 16) >> 4) << 4;
        const auto encOffset = 0;

        // protocol 3.3 - encrypted data only
        //const auto encLen = ((length + 16) >> 4) << 4;
        //const auto encOffset = 15;

        //
        if (!output) output = (uint8_t*)calloc(1, encLen + encOffset);
        memcpy(output, protocolVersion, 3);
        // [4...15) i.e. 12 bytes - something
        memcpy(output + 15, data, length);

        // return encryptable zone
        return std::array<uint32_t, 2>{encOffset, encLen};
    }
};

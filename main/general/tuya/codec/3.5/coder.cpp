#include "../libtuya.hpp"

//
namespace tc {
    size_t prepareTuyaCode35(size_t& encrypted_length, TuyaCmd const& cmdDesc, uint8_t* output) {
        // write header
        *(uint32_t*)(output+0) = bswap32(0x00006699);

        // encode as big-endian
        *(uint16_t*)(output+4)  = 0u;
        *(uint32_t*)(output+6)  = bswap32(cmdDesc.SEQ_NO);
        *(uint32_t*)(output+10) = bswap32(cmdDesc.CMD_ID);
        *(uint32_t*)(output+14) = bswap32(encrypted_length);

        // all of those is a part encryption/encoding stage
        // before payload goes 12-bytes IV
        // after payload goes 16-bytes AES-GCM tag
        const uint32_t header_len = 18;
        *(uint32_t*)(output + header_len + encrypted_length) = bswap32(0x00009966);
        return ((encrypted_length + header_len) + 4);
    }

    //
    size_t encodeTuyaCode35(uint8_t* encrypted_data, size_t& encrypted_length, TuyaCmd const& cmdDesc, uint8_t* output) {
        prepareTuyaCode35(encrypted_length, cmdDesc, output);
        memcpy(output + 18, encrypted_data, encrypted_length);
        return ((encrypted_length + 18) + 4);
    }
};

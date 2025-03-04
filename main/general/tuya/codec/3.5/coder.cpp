#include "../libtuya.hpp"

//
namespace tc {
    const uint32_t header_len = 18;

    // all of those is a part encryption/encoding stage
    // before payload goes 12-bytes IV
    // after payload goes 16-bytes AES-GCM tag
    bytespan_t prepareTuyaCode35(size_t const& gcm_iv_data_tag_size, TuyaCmd const& cmdDesc,  bytes_t& output) {
        *reinterpret_cast<uint32_t*>(output->data() + 0)  = bswap32(0x00006699);
        *reinterpret_cast<uint16_t*>(output->data() + 0)  = bswap16(0); // CRC16 (not implemented)
        *reinterpret_cast<uint32_t*>(output->data() + 6)  = bswap32(cmdDesc.SEQ_NO);
        *reinterpret_cast<uint32_t*>(output->data() + 10) = bswap32(cmdDesc.CMD_ID);
        *reinterpret_cast<uint32_t*>(output->data() + 14) = bswap32(gcm_iv_data_tag_size);
        *reinterpret_cast<uint32_t*>(output->data() + (header_len + gcm_iv_data_tag_size)) = bswap32(0x00009966);
        return bytespan_t(output->data(), (header_len + gcm_iv_data_tag_size) + 4);
    }

    //
    bytespan_t encodeTuyaCode35(bytespan_t const& gcm_iv_data_tag, TuyaCmd const& cmdDesc, bytes_t& output) {
        auto out = prepareTuyaCode35(gcm_iv_data_tag, cmdDesc, output);
        memcpy(out->data() + header_len, gcm_iv_data_tag->data(), gcm_iv_data_tag->size());
        return out;
    }
};

#include "../libtuya.hpp"

//
namespace tc {
    // HMAC i.e. hmac_key
    bytespan_t prepareTuyaCode(size_t const& length, TuyaCmd const& cmdDesc,  bytes_t& output) {
        // encode as big-endian
        const auto headerLen   = 16;
        const auto payloadSize = computePayloadSize(length, cmdDesc.HMAC ? true : false);

        // write header
        *reinterpret_cast<uint32_t*>(output->data()+0)  = bswap32(0x000055AA);
        *reinterpret_cast<uint32_t*>(output->data()+4)  = bswap32(cmdDesc.SEQ_NO);
        *reinterpret_cast<uint32_t*>(output->data()+8)  = bswap32(cmdDesc.CMD_ID);
        *reinterpret_cast<uint32_t*>(output->data()+12) = bswap32(payloadSize);

        //
        auto payload = output->data() + headerLen;
        const auto dataLen = (length + (cmdDesc.HMAC ? 32 : 4));
        for (uint i=0;i<dataLen;i++) { payload[i] = 0; }

        // write suffix
        *reinterpret_cast<uint32_t*>(payload + dataLen) = bswap32(0x0000AA55);
        return bytespan_t(output->data(), payloadSize + headerLen);
    }

    // HMAC i.e. hmac_key
    bytespan_t encodeTuyaCode(bytespan_t const& encrypted_data, TuyaCmd const& cmdDesc,  bytes_t& output) {
        auto out = prepareTuyaCode(encrypted_data->size(), cmdDesc, output);
        memcpy(out->data() + 16, encrypted_data->data(), encrypted_data->size());
        return checksumTuyaCode(out, cmdDesc.HMAC);
    }
};

#include "../session.hpp"

//
namespace th {
    //
    void TuyaSession::init(std::string tuya_local_key, std::string device_id, std::string device_uid) {
        this->tuya_local_ip  = tuya_local_ip;
        this->tuya_local_key = tuya_local_key;
        this->device_id  = device_id;
        this->device_uid = device_uid;

        // padding from IV
        //hmac_key = hmac_payload + 12;

        // parse from string and use it
        //uint8_t ip[4] = {0, 0, 0, 0};
        //ipv4_parse((uint8_t *)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);

        //
        //memcpy(hmac_key, tuya_local_key.c_str(), 16);
        SEQ_NO = 1;
        linked = false;
    }

    //
    bytespan_t TuyaSession::handleSignal(bytespan_t const& inBuffer) {
        auto payload = tc::getTuyaPayload(inBuffer);
        const auto code  = tc::getTuyaCmd(payload->data());
        if (code == 0x4) { auto nonce = *reinterpret_cast<bigint_t const*>(payload->data()); auto ptr = encodeMessage(0x5u, sharedNonce(nonce)); resolveKey(nonce); return ptr; } else
        if (code == 0x8) { handleJson(payload); };
        return payload;
    }
};

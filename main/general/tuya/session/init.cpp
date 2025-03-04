#include "../session.hpp"

//
namespace th {
    //
    void TuyaSession::init(std::string device_id, std::string tuya_local_key, std::string device_uid) {
        this->tuya_local_key = tuya_local_key;
        this->device_id  = device_id;
        this->device_uid = device_uid;
        this->hmac_key = *reinterpret_cast<tc::block_t const*>(this->tuya_local_key.c_str());

        // parse from string and use it
        //uint8_t ip[4] = {0, 0, 0, 0};
        //ipv4_parse((uint8_t *)tuya_local_ip.c_str(), tuya_local_ip.size(), ip);

        //
        SEQ_NO = 1;
        linked = false;
    }

    //
    bytespan_t TuyaSession::handleSignal(bytespan_t const& inBuffer) {
        const auto payload = tc::getTuyaPayload(inBuffer);
        const auto code    = tc::getTuyaCmd(inBuffer->data());
        if (code == 0x4) { auto nonce = *reinterpret_cast<tc::block_t const*>(payload->data()); auto ptr = encodeMessage(0x5u, hmac = sharedNonce(nonce)); hmac_key = resolveKey(nonce); return ptr; } else
        if (code == 0x8) { handleJson(payload); };
        return {};
    }
};

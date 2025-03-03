#ifndef CD0F5375_8473_4939_A778_8FD897C00B47
#define CD0F5375_8473_4939_A778_8FD897C00B47

//
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>

//
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>
#include <std/types.hpp>

//
#include "./PASE/spake2p.hpp"

//
#include "../codec/message/Message.hpp"
#include "../tlv/parts/enums.hpp"
#include "../tlv/tlv_tree.h"
#include "../tlv/tlv.h"
#include "../channel/channel.hpp"

//
struct PBKDFParamRequest { bigint_t rand = 0; uint16_t sess = 0; uint8_t pass = 0; };

//
class PASE {
public:
    inline void init() { channel = Channel(); }
    inline PASE() { init(); }
    inline PASE(Channel const& channel) : channel(channel) {  }

    //
    SessionKeys& makeSessionKeys();
    bytespan_t makePASEResponse(Message const& request);
    bytespan_t makePAKE2(Message const& request);

    //
    uint8_t handlePASERequest(Payload const& payload);
    uint8_t handlePAKE1(Payload const& payload);
    uint8_t handlePAKE3(Payload const& payload);

    //
    Channel makeChannel() { return Channel(makeSessionKeys()); }
    Channel& getChannel() { return channel; }
    Channel const& getChannel() const { return channel; }

    //
    inline uint8_t handlePayload(Payload const& payload) {
        switch (payload.header.protocolCode) {
            case 0x20: return handlePASERequest(payload);
            case 0x22: return handlePAKE1(payload);
            case 0x24: return handlePAKE3(payload);
            default: return 0;
        }
        return payload.header.protocolCode;
    }

    //
private: //
    HKDF_HMAC hkdf = {};
    PBKDFParameters params = {};
    PBKDFParamRequest req = {};
    Channel channel = {};

    //
    ecp_t X_;
    std::shared_ptr<Spake2p> spake = {};
    RNG rng = {};
};

#endif /* CD0F5375_8473_4939_A778_8FD897C00B47 */

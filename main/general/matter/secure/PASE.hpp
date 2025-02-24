#ifndef C89D871D_0511_4554_A58E_A06935CE9861
#define C89D871D_0511_4554_A58E_A06935CE9861

//
#include <spake2p/spake2p.hpp>

//
#include <tlv/parts/enums.hpp>
#include <tlv/tlv_tree.h>
#include <tlv/tlv.h>

//
#include <std/types.hpp>

//
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>

//
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>

//
#include "./PASE/structs.hpp"
#include "../codec/message/Message.hpp"

//
class PASE {
public:
    inline void init() { counter = rng.generate(); }
    inline PASE() { init(); }

    //
    bytespan_t makeAckMessage(Message const& request);
    bytespan_t makeReportStatus(Message const& request, uint16_t const& status = 0);
    bytespan_t makePASEResponse(Message const& request);
    bytespan_t makePAKE2(Message const& request);

    //
    uint8_t handlePASERequest(Payload const& payload);
    uint8_t handlePAKE1(Payload const& payload);
    uint8_t handlePAKE3(Payload const& payload);
    uint8_t handlePayload(Payload const& payload);

    //
    Message makeMessage(Message const& request, uint8_t messageType, bytespan_t const& payload = {});
    Message decodeMessage(bytespan_t const& bytes) const;
    Message& decryptPayload(Message& message,  bytespan_t const& bytes = {}) const;
    SessionKeys& makeSessionKeys();

    //
private: //
    HKDF_HMAC hkdf = {};
    PBKDFParameters params = {};
    PBKDFParamRequest req = {};
    SessionKeys sessionKeys = {};

    //
    ecp_t X_;
    std::shared_ptr<Spake2p> spake = {};

    //
    uintptr_t counter = 0;
    RNG rng = {};
};

//
#endif

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
#include "../codec/message/Message.hpp"
#include "../tlv/parts/enums.hpp"
#include "../tlv/tlv_tree.h"
#include "../tlv/tlv.h"

//
#include "./core/session.hpp"

//
struct PBKDFParamRequest { bigint_t rand = 0; uint16_t sess = 0; uint8_t pass = 0; };

//
enum ProtocolCode : uint32_t {
    AckMessage = 0x10,
    PASERequest = 0x20,
    PASEResponse = 0x21,
    PASEPake1 = 0x22,
    PASEPake2 = 0x23,
    PASEPake3 = 0x24,
    ReportStatus = 0x40
};

//
class PASE {
public:
    inline void init() { session = Session(); status_ = -1; }
    inline PASE() { init(); }
    inline PASE(Session const& session) : session(session), status_(-1) {  }

    //
    bytespan_t makePASEResponse(Message const& request);
    bytespan_t makePAKE2(Message const& request);
    bytespan_t makeReportStatus(Message const& request, uint16_t const& status = 0);
    bytespan_t makeAckMessage(Message const& request);
    SessionKeys& makeSessionKeys();

    //
    uint8_t handlePASERequest(Payload const& payload);
    uint8_t handlePAKE1(Payload const& payload);
    uint8_t handlePAKE3(Payload const& payload);

    //
    inline Session makeSession() { return Session(session, makeSessionKeys(), req.sess); }
    inline Session& getSession() { return session; }
    inline Session const& getSession() const { return session; }
    inline int const& status() const { return status_; }
    inline bytespan_t handleMessage(Message const& message) {
        switch (message.decodedPayload.header.protocolCode) {
            case ProtocolCode::PASERequest: { handlePASERequest(message.decodedPayload); return makePASEResponse(message); break; };
            case ProtocolCode::PASEPake1: { handlePAKE1(message.decodedPayload); return makePAKE2(message); break; };
            case ProtocolCode::PASEPake3: { handlePAKE3(message.decodedPayload); return makeReportStatus(message, 0); };
            default: return {};
        }
        return {};
    }

    //
private: //
    HKDF_HMAC hkdf = {};
    PBKDFParameters params = {};
    PBKDFParamRequest req = {};
    Session session = {};
    int status_ = -1;

    //
    ecp_t X_;
    std::shared_ptr<Spake2p> spake = {};
    RNG rng = {};
};

//
#endif

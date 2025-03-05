#ifndef C89D871D_0511_4554_A58E_A06935CE9861
#define C89D871D_0511_4554_A58E_A06935CE9861

//
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>

//
#include <std/types.hpp>

//
#include "../../codec/message/Message.hpp"
#include "../../tlv/parts/enums.hpp"
#include "../../tlv/tlv_tree.h"
#include "../../tlv/tlv.h"

//
#include "./structs.hpp"



//
class Session {
public:
    inline void init() { counter = rng.generate(); }
    inline Session(uint16_t const& sessionId = 0) : sessionId(sessionId) { init(); }
    inline Session(Session const& channel, uint16_t const& sessionId = 0) : sessionId(sessionId), sessionKeys(channel.sessionKeys), counter(channel.counter), rng(channel.rng) {  }
    inline Session(Session const& channel, SessionKeys const& sessionKeys, uint16_t const& sessionId = 0) : sessionId(sessionId), sessionKeys(sessionKeys), counter(channel.counter), rng(channel.rng)  {  }
    inline Session(SessionKeys const& sessionKeys, uint16_t const& sessionId = 0) : sessionId(sessionId), sessionKeys(sessionKeys) { counter = rng.generate(); }

    //
    Session& operator =(Session const& channel) {
        sessionKeys = channel.sessionKeys;
        counter = channel.counter;
        return *this;
    }

    //
    Message makeMessage(Message const& request, uint8_t messageType, bytespan_t const& payload = {});
    Message makeMessage(Message const& request, uint8_t messageType, tlvcpp::tlv_tree_node const& payload);
    Message decodeMessage(bytespan_t const& bytes) const;
    bytespan_t encodeMessage(Message& message) const;
    bytespan_t makeAckMessage(Message const& request);

    //, sessionKeys
    bytespan_t& decryptPayload(Message& message,  bytespan_t const& bytes = {}) const;
    bytespan_t& encryptPayload(Message& message,  bytespan_t const& bytes = {}) const;

    //
    SessionKeys& setSessionKeys(SessionKeys const& sk) { sessionKeys = sk; return sessionKeys; }
    SessionKeys const& getSessionKeys() const { return sessionKeys; }

    //
private: //
    SessionKeys sessionKeys = {};
    uintptr_t counter = 0;
    uint16_t sessionId = 0;
    RNG rng = {};
};

//
#endif

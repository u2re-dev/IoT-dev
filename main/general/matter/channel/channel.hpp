#ifndef C89D871D_0511_4554_A58E_A06935CE9861
#define C89D871D_0511_4554_A58E_A06935CE9861

//
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>

//
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>
#include <raii/misc.hpp>
#include <std/types.hpp>

//
#include "../codec/message/Message.hpp"
#include "../tlv/parts/enums.hpp"
#include "../tlv/tlv_tree.h"
#include "../tlv/tlv.h"

//
#include "./core/structs.hpp"

//
class Channel {
public:
    inline void init() { counter = rng.generate(); }
    inline Channel() { init(); }
    inline Channel(Channel const& channel) : sessionKeys(channel.sessionKeys), counter(channel.counter), rng(channel.rng) {  }
    inline Channel(SessionKeys const& sessionKeys) : sessionKeys(sessionKeys) { counter = rng.generate(); }

    //
    Channel& operator =(Channel const& channel) {
        sessionKeys = channel.sessionKeys;
        counter = channel.counter;
        return *this;
    }

    //
    bytespan_t makeAckMessage(Message const& request);

    //
    Message makeMessage(Message const& request, uint8_t messageType, bytespan_t const& payload = {});
    Message makeMessage(Message const& request, uint8_t messageType, tlvcpp::tlv_tree_node const& payload);
    Message decodeMessage(bytespan_t const& bytes) const;
    bytespan_t& decryptPayload(Message& message,  bytespan_t const& bytes = {}) const;

    //
    SessionKeys& setSessionKeys(SessionKeys const& sk) { sessionKeys = sk; return sessionKeys; }
    SessionKeys const& getSessionKeys() const { return sessionKeys; }

    //
private: //
    SessionKeys sessionKeys = {};
    uintptr_t counter = 0;
    RNG rng = {};
};

//
#endif

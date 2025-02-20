#ifndef C89D871D_0511_4554_A58E_A06935CE9861
#define C89D871D_0511_4554_A58E_A06935CE9861

//
#include "./spake2p/spake2p-p256.hpp"
#include "./matter/codec/message/Message.hpp"
#include "./tlv/tlv_tree.h"
#include "./tlv/tlv.h"

//
#include "std/types.hpp"
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>

//
struct PBKDFParamRequest {
    bigint_t rand;
    uint16_t sess;
    uint16_t pass;
};

//
struct RNG {
    std::random_device dev;
    std::mt19937 rng;

    //
    RNG() : rng(std::mt19937(dev())) {};
    RNG(RNG const& o) : rng(o.rng) {};

    //
    template<typename T = uint64_t> T generate() {
        std::uniform_int_distribution<T> dist;
        return dist(rng);
    }
};



//
class PASE {
public:
    inline void init() { counter = rng.generate(); }
    inline PASE() { init(); };



    //
    inline Message decodeMessage(bytespan_t const& bytes) {
        auto reader  = reader_t(bytes);
        auto message = MessageCodec::decodeMessage (reader);
        auto readerPayload = reader_t(message.rawPayload);
        message.decodedPayload = MessageCodec::decodePayload(readerPayload);
        return message;
    }

    //
    inline uint8_t handlePayload(Payload const& payload) {
        switch (payload.header.messageType) {
            case 0x20: return handlePASERequest(payload);
            case 0x22: return handlePAKE1(payload);
            case 0x24: return handlePAKE3(payload);
            default: return 0;
        }
        return payload.header.messageType;
    }

    //
    inline uint8_t handlePAKE3(Payload const& payload) {
        if (payload.header.messageType != 0x24) return 0;

        //
        auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(*payload.payload);
        auto Xv = tlv.find(tlvcpp::tag_t(01))->data();
        auto hAY = *(bigint_t*)Xv.payload();

        //
        if (hkdf.hAY != hAY) {
            throw std::runtime_error("hAY not match in MAKE3 phase (received value)");
        }

        //
        return payload.header.messageType;
    }

    //
    inline uint8_t handlePAKE1(Payload const& payload) {
        if (payload.header.messageType != 0x22) return 0;

        //
        auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(*payload.payload);
        auto Xv = tlv.find(tlvcpp::tag_t(01))->data();
        X_ = spake->parseECP(Xv.payload(), 65);

        //
        return payload.header.messageType;
    }

    //
    inline uint8_t handlePASERequest(Payload const& payload) {
        if (payload.header.messageType != 0x20) return 0;

        //
        auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(*payload.payload);
        auto sess = tlv.find(tlvcpp::tag_t(02))->data();
        auto pass = tlv.find(tlvcpp::tag_t(03))->data();

        //
        req = PBKDFParamRequest{ *((bigint_t const*)tlv.find(tlvcpp::tag_t(01))->data().payload()), sess, pass };
        params = {1000, mpi_t().random() };

        //
        return payload.header.messageType;
    }





    //
    inline Message makeResponse(Message const& request, uint8_t messageType, bytespan_t const& payload = {}) {
        Message outMsg = {};
        outMsg.header.messageId  = (counter++); ///- request.header.messageId;
        outMsg.header.sessionId  = request.header.sessionId;
        outMsg.header.destNodeId = request.header.sourceNodeId;
        outMsg.decodedPayload.header.messageType = messageType;
        outMsg.decodedPayload.header.requiresAck = false;//true;
        outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
        outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
        outMsg.decodedPayload.header.ackedMessageId  = request.header.messageId;
        outMsg.decodedPayload.payload = payload;
        return outMsg;//MessageCodec::encodeMessage(outMsg);
    }

    //
    inline bytespan_t makeAckMessage(Message const& request) {
        Message outMsg = makeResponse(request, 0x10);
        return MessageCodec::encodeMessage(outMsg);
    }



    //
    inline bytespan_t makePAKE2(Message const& request) {
        if (request.decodedPayload.header.messageType != 0x22) return {};

        //
        bytespan_t Y  = spake->computeY();
        hkdf = spake->computeHKDFFromX(X_);

        //
        auto resp = tlvcpp::tlv_tree_node{}; 
        resp.data() = tlvcpp::tlv(00,  0, nullptr, 0x15);
        resp.add_child(tlvcpp::tlv(01, 65, Y->data(), 0x10)); // send randoms
        resp.add_child(tlvcpp::tlv(02, 32, (uint8_t*)&hkdf.hBX, 0x10));

        //
        writer_t respTLV; resp.serialize(respTLV);
        Message outMsg = makeResponse(request, 0x23, respTLV);
        return MessageCodec::encodeMessage(outMsg);
    }

    //
    inline bytespan_t makePASEResponse(Message const& request) {
        if (request.decodedPayload.header.messageType != 0x20) return {};

        //
        bigint_t rand = mpi_t().random();
        auto resp = tlvcpp::tlv_tree_node{}; 
        resp.data() = tlvcpp::tlv(00,  0, nullptr, 0x15);
        resp.add_child(tlvcpp::tlv(01, 32, (uint8_t*)&req.rand, 0x10)); // send randoms
        resp.add_child(tlvcpp::tlv(02, 32, (uint8_t*)&rand, 0x10));
        resp.add_child(tlvcpp::tlv(03,  uint16_t(rng.generate())));

        //
        auto& tlvParams = resp.add_child(tlvcpp::tlv(04, 0, nullptr, 0x15));;
        tlvParams.add_child(tlvcpp::tlv(01, params.iterations));
        tlvParams.add_child(tlvcpp::tlv(02, 32, (uint8_t*)&params.salt, 0x10));

        //
        writer_t respTLV; resp.serialize(respTLV); spake = std::make_shared<Spake2p>(params, req.pass, Spake2p::computeContextHash(request.decodedPayload.payload, respTLV));
        Message outMsg = makeResponse(request, 0x21, respTLV);
        return MessageCodec::encodeMessage(outMsg);
    }



    //
private: //
    HKDF_HMAC hkdf = {};
    PBKDFParameters params = {};
    PBKDFParamRequest req = {};

    //
    ecp_t X_;
    std::shared_ptr<Spake2p> spake = {};

    //
    uintptr_t counter = 0;
    RNG rng = {};
};

//
#endif

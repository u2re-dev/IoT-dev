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

//
struct PBKDFParamRequest {
    bigint_t rand;
    uint16_t sess;
    uint16_t pass;
};

//
inline bytes_t encode_PBKDFParamResponse(PbkdfParameters const& parameter = {}, 
    bigint_t const& reqRandom = 0,
    bigint_t const& resRandom = 0
) {
    auto resp = tlvcpp::tlv_tree_node{}; 
    resp.data() = tlvcpp::tlv(00,  0, nullptr, 0x15);
    resp.add_child(tlvcpp::tlv(01, 32, (uint8_t*)&reqRandom, 0x10)); // send randoms
    resp.add_child(tlvcpp::tlv(02, 32, (uint8_t*)&resRandom, 0x10));

    //
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist;
    resp.add_child(tlvcpp::tlv(03,  uint16_t(dist(rng))));

    //
    auto& params = resp.add_child(tlvcpp::tlv(04, 0, nullptr, 0x15));;
    params.add_child(tlvcpp::tlv(01, parameter.iterations));

    //
    bytes_t salt = make_bytes((uint8_t*)&parameter.salt, (uint8_t*)&parameter.salt + 32);

    //
    params.add_child(tlvcpp::tlv(02, salt->size(), salt->data(), 0x10));
    writer_t respTLV; resp.serialize(respTLV); return respTLV;
};

//
class Matter {
public:

    inline void init() {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist;
        counter = dist(rng);
    }

    //
    inline Message decodeUnencryptedMessage(bytes_t const& bytes) {
        auto reader  = reader_t(bytes);
        auto message = MessageCodec::decodeMessage (reader);
        auto readerPayload = reader_t(message.rawPayload);
        message.decodedPayload = MessageCodec::decodePayload(readerPayload);
        return message;
    }

    //
    inline bytes_t makeAckMessage(Message const& request) {
        Message outMsg = {};

        //
        //auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        //if (timestamp != counter) { counter = timestamp; } else { counter++; };
        outMsg.header.messageId  = (counter++); ///- request.header.messageId;
        outMsg.header.sessionId  = request.header.sessionId;
        outMsg.header.destNodeId = request.header.sourceNodeId;
        outMsg.decodedPayload.header.messageType = 0x10;
        outMsg.decodedPayload.header.requiresAck = false;//true;
        outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
        outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
        outMsg.decodedPayload.header.ackedMessageId  = request.header.messageId;
        return MessageCodec::encodeMessage(outMsg);
    }

    //
    inline bytes_t makePAKE2(Message const& request) {
        if (request.decodedPayload.header.messageType != 0x22) return {};

        // hold with Y, X is dedicated
        //auto Y = spake->computeY();
        auto Y = spake->computeY();
        auto Ve = spake->computeSecretAndVerifiersFromX(X_);

        //
        auto resp = tlvcpp::tlv_tree_node{}; 
        resp.data() = tlvcpp::tlv(00,  0, nullptr, 0x15);
        resp.add_child(tlvcpp::tlv(01, 65, Y->data(), 0x10)); // send randoms
        resp.add_child(tlvcpp::tlv(02, 32, (uint8_t*)&Ve.hBX, 0x10));
        writer_t respTLV; resp.serialize(respTLV);

        //
        Message outMsg = {};
        outMsg.header.messageId  = (counter++); //- request.header.messageId;
        outMsg.header.sessionId  = request.header.sessionId;
        outMsg.header.destNodeId = request.header.sourceNodeId;
        outMsg.decodedPayload.header.requiresAck = false;
        outMsg.decodedPayload.header.messageType = 0x23;
        outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
        outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
        outMsg.decodedPayload.header.ackedMessageId  = request.header.messageId;
        outMsg.decodedPayload.payload = respTLV;
        return MessageCodec::encodeMessage(outMsg);
    }

    //
    inline std::optional<ecp_t> handlePAKE1(Payload const& payload) {
        if (payload.header.messageType != 0x22) return {};

        //
        auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(*payload.payload);
        auto Xv = tlv.find(tlvcpp::tag_t(01))->data();
        return (X_ = spake->parseECP(Xv.payload(), 65));
    }

    //
    inline std::optional<PBKDFParamRequest> handlePAKERequest(Payload const& payload) {
        if (payload.header.messageType != 0x20) return {};
        auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(*payload.payload);

        // parse PBKDFParamRequest
        auto sess = tlv.find(tlvcpp::tag_t(02))->data();
        auto pass = tlv.find(tlvcpp::tag_t(03))->data();

        //
        req = PBKDFParamRequest{ 0, sess, pass };
        memcpy(&req.rand, tlv.find(tlvcpp::tag_t(01))->data().payload(), 32);
        parameters = {1000, mpi_t().random() };

        //
        return req;
    }

    //
    inline bytes_t makePAKEResponse(Message const& request) {
        if (request.decodedPayload.header.messageType != 0x20) return {};
        Message outMsg = {};

        //
        //auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        //if (timestamp != counter) { counter = timestamp; } else { counter++; };
        outMsg.header.messageId  = (counter++); //- request.header.messageId;
        outMsg.header.sessionId  = request.header.sessionId;
        outMsg.header.destNodeId = request.header.sourceNodeId;
        outMsg.decodedPayload.header.requiresAck = false;
        outMsg.decodedPayload.header.messageType = 0x21;
        outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
        outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
        outMsg.decodedPayload.header.ackedMessageId  = request.header.messageId;
        outMsg.decodedPayload.payload = encode_PBKDFParamResponse(parameters, req.rand, mpi_t().random());
        auto msg = MessageCodec::encodeMessage(outMsg);
        spake = std::make_shared<Spake2p>(parameters, req.pass, computeContextHash(request.decodedPayload.payload, outMsg.decodedPayload.payload));
        return msg;
    }

private:
    PBKDFParamRequest req = {};

    //
    ecp_t X_;
    uintptr_t counter = 0;//std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();//1;
    PbkdfParameters parameters = {};
    std::shared_ptr<Spake2p> spake = {};
};

//
#endif

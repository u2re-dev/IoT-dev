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
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>

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
struct SessionKeys {
    intx::uint128 I2Rkeys;
    intx::uint128 R2Ikeys;
    intx::uint128 AttestationChallenge;
};


//
class PASE {
public:
    inline void init() { counter = rng.generate(); }
    inline PASE() { init(); }

    //
    inline Message decodeMessage(bytespan_t const& bytes) const {
        auto reader  = reader_t(bytes);
        auto message  = MessageCodec::decodeMessage(reader);

        //
        if (message.header.sessionId != 0 && message.rawPayload->size() && sessionKeys.I2Rkeys) {
            auto aad = std::span<uint8_t>(bytes->begin(), bytes->end() - message.rawPayload->size());

            //
            writer_t nonce_w;
            nonce_w.writeByte(message.header.securityFlags);
            nonce_w.writeUInt32(message.header.messageId);
            nonce_w.writeUInt64(message.header.sourceNodeId.value_or(0));
            bytespan_t nonce = nonce_w;

            //
            mbedtls_ccm_context aes;
            mbedtls_ccm_init(&aes);

            // set keys
            checkMbedtlsError(mbedtls_ccm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, (uint8_t*) &sessionKeys.I2Rkeys, 128), "Decrypt Key Init Failed");

            //
            auto output = make_bytes(message.rawPayload->size() - 16);
            checkMbedtlsError(mbedtls_ccm_auth_decrypt(&aes, //MBEDTLS_CCM_DECRYPT, 
                (message.rawPayload->size() - 16),
                nonce->data(),
                nonce->size(),
                aad.data(),
                aad.size(),
                //message.rawPayload->size() - 16,//aad.size(),
                message.rawPayload->data(),
                output->data(),
                message.rawPayload->data() + (message.rawPayload->size() - 16),
                16
            ), "Decryption Failed");
            mbedtls_ccm_free(&aes);

            std::cout << "Decoded Payload:" << std::endl;
            std::cout << hex::b2h(output) << std::endl;
    
        }

        //
        auto readerPayload = reader_t(message.rawPayload);//message.rawPayload
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
        if (hkdf.hAY != hAY) { throw std::runtime_error("hAY not match in MAKE3 phase (received value)"); }

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
    inline Message makeMessage(Message const& request, uint8_t messageType, bytespan_t const& payload = {}) {
        Message outMsg = {};
        outMsg.header.messageId  = (counter++); ///- request.header.messageId;
        outMsg.header.sessionId  = request.header.sessionId;
        outMsg.header.destNodeId = request.header.sourceNodeId;
        outMsg.decodedPayload.header.messageType = messageType;
        outMsg.decodedPayload.header.requiresAck = false;//true;
        outMsg.decodedPayload.header.exchangeId  = request.decodedPayload.header.exchangeId;
        outMsg.decodedPayload.header.protocolId  = request.decodedPayload.header.protocolId;
        outMsg.decodedPayload.header.ackedMessageId = request.header.messageId;
        outMsg.decodedPayload.payload = payload;
        return outMsg;//MessageCodec::encodeMessage(outMsg);
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
        Message outMsg = makeMessage(request, 0x23, respTLV);
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
        Message outMsg = makeMessage(request, 0x21, respTLV);
        return MessageCodec::encodeMessage(outMsg);
    }



    //
    inline bytespan_t makeAckMessage(Message const& request) {
        Message outMsg = makeMessage(request, 0x10);
        return MessageCodec::encodeMessage(outMsg);
    }

    //
    inline bytespan_t makeReportStatus(Message const& request, uint16_t const& status = 0) {
        makeSessionKeys();

        //
        Message outMsg = makeMessage(request, 0x40, make_bytes(8));
        *(uint16_t*)(outMsg.decodedPayload.payload->data()+0) = 0;
        *(uint32_t*)(outMsg.decodedPayload.payload->data()+2) = request.decodedPayload.header.protocolId;
        *(uint16_t*)(outMsg.decodedPayload.payload->data()+6) = status;
        return MessageCodec::encodeMessage(outMsg);
    }

    //
    inline SessionKeys makeSessionKeys() {
        auto info = hex::s2b("SessionKeys");
        auto keys = crypto::hkdf_len(hkdf.Ke, info);
        return (sessionKeys = SessionKeys {
            *(intx::uint128*)(keys->data()),
            *(intx::uint128*)(keys->data() + 16),
            *(intx::uint128*)(keys->data() + 32)
        });
    }



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

#ifndef A6437079_C7FE_4DEC_B6BE_1544804D63B7
#define A6437079_C7FE_4DEC_B6BE_1544804D63B7

//
#include "../PASE.hpp"

//
uint8_t PASE::handlePASERequest(Payload const& payload) {
    if (payload.header.messageType != 0x20) return 0;

    //
    auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(payload.payload);
    req = PBKDFParamRequest{ tlv.find(01)->data(), tlv.find(02)->data(), tlv.find(03)->data() };
    params = {1000, mpi_t().random() };

    //
    return payload.header.messageType;
}

//
uint8_t PASE::handlePAKE1(Payload const& payload) {
    if (payload.header.messageType != 0x22) return 0;

    //
    auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(payload.payload);
    X_ = spake->parseECP(tlv.find(01)->data().payload(), 65);

    //
    return payload.header.messageType;
}

//
uint8_t PASE::handlePAKE3(Payload const& payload) {
    if (payload.header.messageType != 0x24) return 0;

    //
    auto tlv = tlvcpp::tlv_tree_node{}; tlv.deserialize(payload.payload);
    bigint_t hAY = tlv.find(01)->data(); // TODO: fix bigint conversion (directly)
    if (hkdf.hAY != hAY) { throw std::runtime_error("hAY not match in MAKE3 phase (received value)"); }

    //
    return payload.header.messageType;
}

#endif /* A6437079_C7FE_4DEC_B6BE_1544804D63B7 */

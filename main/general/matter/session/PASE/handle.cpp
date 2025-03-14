#ifndef A6437079_C7FE_4DEC_B6BE_1544804D63B7
#define A6437079_C7FE_4DEC_B6BE_1544804D63B7

//
#include "../PASE.hpp"

//
uint8_t PASE::handlePASERequest(Payload const& payload) {
    if (payload.header.protocolCode != ProtocolCode::PASERequest) return 0;
    req = PBKDFParamRequest{
        payload.TLV.find(01)->data(),
        payload.TLV.find(02)->data(),
        payload.TLV.find(03)->data()
    };
    params = {1000, mpi_t().random() };
    return payload.header.protocolCode;
}

//
uint8_t PASE::handlePAKE1(Payload const& payload) {
    if (payload.header.protocolCode != ProtocolCode::PASEPake1) return 0;
    X_ = spake->parseECP(payload.TLV.find(01)->data().payload(), 65);
    return payload.header.protocolCode;
}

//
uint8_t PASE::handlePAKE3(Payload const& payload) {
    if (payload.header.protocolCode != ProtocolCode::PASEPake3) return 0;
    bigint_t hAY = payload.TLV.find(01)->data(); // TODO: fix bigint conversion (directly)
    if (hkdf.hAY != hAY) { throw std::runtime_error("hAY not match in MAKE3 phase (received value)"); }
    status_ = 0; return payload.header.protocolCode;
}

//
#endif

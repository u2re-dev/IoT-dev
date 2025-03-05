#ifndef E0967858_9A99_411B_AE86_8645186140D5
#define E0967858_9A99_411B_AE86_8645186140D5

//
#include "../PASE.hpp"

//
bytespan_t PASE::makePASEResponse(Message const& request) {
    if (request.decodedPayload.header.protocolCode != ProtocolCode::PASERequest) return {};

    // TODO: fix memory lost
    bigint_t rand = mpi_t().random();
    auto resp = tlvcpp::tlv_tree_node{};
    resp.data() = tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0};
    resp.add_child(req.rand, 01); // send randoms
    resp.add_child(rand, 02);     // will lost if pass directly (without variable declaration)
    resp.add_child(uint16_t(rng.generate()), 03);

    //
    decltype(auto) secp = resp.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0}, 04);
    secp.add_child(params.iterations, 01);
    secp.add_child(params.salt, 02);

    //
    Message outMsg = session.makeMessage(request, ProtocolCode::PASEResponse, resp);
    decltype(auto) encoded = session.encodeMessage(outMsg);

    //
    spake = std::make_shared<Spake2p>(params, req.pass, Spake2p::computeContextHash(request.decodedPayload.payload, outMsg.decodedPayload.payload));
    return encoded;
}

//
bytespan_t PASE::makePAKE2(Message const& request) {
    if (request.decodedPayload.header.protocolCode != ProtocolCode::PASEPake1) return {};

    //
    auto resp = tlvcpp::tlv_tree_node{};
    resp.data() = tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0};
    resp.add_child(spake->computeY().toBytes(), 01);  // works only when Y stored with `spake`
    resp.add_child((hkdf = spake->computeHKDFFromX(X_)).hBX, 02);

    //
    Message outMsg = session.makeMessage(request, ProtocolCode::PASEPake2, resp);
    return session.encodeMessage(outMsg);
}

//
bytespan_t PASE::makeReportStatus(Message const& request, uint16_t const& status) {
    Message outMsg = session.makeMessage(request, ProtocolCode::ReportStatus, make_bytes(8));
    *reinterpret_cast<uint16_t*>(outMsg.decodedPayload.payload->data()+0) = 0;
    *reinterpret_cast<uint32_t*>(outMsg.decodedPayload.payload->data()+2) = request.decodedPayload.header.protocolId;
    *reinterpret_cast<uint16_t*>(outMsg.decodedPayload.payload->data()+6) = status;
    return session.encodeMessage(outMsg);
}

//
bytespan_t PASE::makeAckMessage(Message const& request) {
    return session.makeAckMessage(request, ProtocolCode::AckMessage);
}

//
SessionKeys& PASE::makeSessionKeys() {
    auto info = hex::s2b("SessionKeys");
    auto keys = crypto::hkdf_len(hkdf.Ke, info);

    // TODO: better interpret code
    return (session.setSessionKeys(SessionKeys {
        *reinterpret_cast<intx::uint128*>(keys->data()),
        *reinterpret_cast<intx::uint128*>(keys->data() + 16),
        *reinterpret_cast<intx::uint128*>(keys->data() + 32)
    }));
}

//
#endif

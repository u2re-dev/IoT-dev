#ifndef E0967858_9A99_411B_AE86_8645186140D5
#define E0967858_9A99_411B_AE86_8645186140D5

//
#include "../PASE.hpp"


//
tlvcpp::tlv_tree_node PASE::makeByPath(tlvcpp::tlv_tree_node const& path) {
    auto attrib  = tlvcpp::tlv_tree_node(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0});
    auto& status = attrib.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 1}, 0);
    auto& data   = attrib.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 1}, 1);
    data.add_child(path.data().control(), 1);
    if (uint32_t(path.find(4)->data()) == 4) { data.add_child(true, 2); }; // TODO: use data per path's

    // TODO: status
    //status.add_child(path.data().control(), 1);

    return attrib;
}

//
bytespan_t PASE::makeReportDataMessage(Message const& request) {
    auto resp   = tlvcpp::tlv_tree_node{};
    resp.data() = tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0};

    //
    auto attrib = resp.add_child(tlvcpp::control_t{2, tlvcpp::e_type::STRUCTURE, 1}, 1);
    for (auto& path : request.decodedPayload.TLV.find(0)->children()) {
        attrib.add_child(makeByPath(path));
    }

    //
    Message outMsg = makeMessage(request, 0x05, resp);
    return MessageCodec::encodeMessage(outMsg);
}




//
bytespan_t PASE::makePASEResponse(Message const& request) {
    if (request.decodedPayload.header.protocolCode != 0x20) return {};

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
    Message outMsg = makeMessage(request, 0x21, resp);
    auto encoded = MessageCodec::encodeMessage(outMsg); // before sending, make spake keys
    spake = std::make_shared<Spake2p>(params, req.pass, Spake2p::computeContextHash(request.decodedPayload.payload, outMsg.decodedPayload.payload));
    return encoded;
}

//
bytespan_t PASE::makePAKE2(Message const& request) {
    if (request.decodedPayload.header.protocolCode != 0x22) return {};

    //
    auto resp = tlvcpp::tlv_tree_node{};
    resp.data() = tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0};
    resp.add_child(spake->computeY().toBytes(), 01);  // works only when Y stored with `spake`
    resp.add_child((hkdf = spake->computeHKDFFromX(X_)).hBX, 02);

    //
    Message outMsg = makeMessage(request, 0x23, resp);
    return MessageCodec::encodeMessage(outMsg);
}



//
bytespan_t PASE::makeAckMessage(Message const& request) {
    Message outMsg = makeMessage(request, 0x10);
    return MessageCodec::encodeMessage(outMsg);
}

//
bytespan_t PASE::makeReportStatus(Message const& request, uint16_t const& status) {
    makeSessionKeys();

    //
    Message outMsg = makeMessage(request, 0x40, make_bytes(8));
    *reinterpret_cast<uint16_t*>(outMsg.decodedPayload.payload->data()+0) = 0;
    *reinterpret_cast<uint32_t*>(outMsg.decodedPayload.payload->data()+2) = request.decodedPayload.header.protocolId;
    *reinterpret_cast<uint16_t*>(outMsg.decodedPayload.payload->data()+6) = status;
    return MessageCodec::encodeMessage(outMsg);
}

//
#endif

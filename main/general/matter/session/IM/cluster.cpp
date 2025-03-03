#include "../IM.hpp"

//
tlvcpp::tlv_tree_node Cluster::makeByPath(tlvcpp::tlv_tree_node const& path) {
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
bytespan_t Cluster::makeReportDataMessage(Message const& request) {
    auto resp   = tlvcpp::tlv_tree_node{};
    resp.data() = tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0};

    //
    auto attrib = resp.add_child(tlvcpp::control_t{2, tlvcpp::e_type::STRUCTURE, 1}, 1);
    for (auto& path : request.decodedPayload.TLV.find(0)->children()) {
        attrib.add_child(makeByPath(path));
    }

    //
    Message outMsg = session.makeMessage(request, 0x05, resp);
    return MessageCodec::encodeMessage(outMsg);
}

//
bytespan_t Cluster::makeReportStatus(Message const& request, uint16_t const& status) {
    Message outMsg = session.makeMessage(request, 0x40, make_bytes(8));
    *reinterpret_cast<uint16_t*>(outMsg.decodedPayload.payload->data()+0) = 0;
    *reinterpret_cast<uint32_t*>(outMsg.decodedPayload.payload->data()+2) = request.decodedPayload.header.protocolId;
    *reinterpret_cast<uint16_t*>(outMsg.decodedPayload.payload->data()+6) = status;
    return MessageCodec::encodeMessage(outMsg);
}

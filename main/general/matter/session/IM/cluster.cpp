#include "../IM.hpp"

//
tlvcpp::tlv_tree_node Cluster::makeByPath(tlvcpp::tlv_tree_node const& path) {
    auto attrib  = tlvcpp::tlv_tree_node(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0});
    auto& status = attrib.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 1}, 0);
    auto& data   = attrib.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 1}, 1);

    // use same path structure (but with redirection)
    { auto redir = path; redir.data() = tlvcpp::tlv(path.data(), 1); data.add_child(redir); };

    // General Commissioning Cluster
    if (uint32_t(path.find(3)->data()) == 0x30) {
        switch (uint32_t(path.find(4)->data())) {
            case 0: data.add_child(uint64_t(0), 2); break;
            case 1: data.add_child(uint16_t(0), 2); break;
            case 2: data.add_child(uint8_t(0), 2); break;
            case 3: data.add_child(uint8_t(0), 2); break;
            case 4: data.add_child(true, 2); break; // same path value
            default:;
        }
    }

    // I don't know what is statuses
    //{ auto redir = path; redir.data() = tlvcpp::tlv(path.data(), 0); status.add_child(redir); }
    //switch (uint32_t(path.find(4)->data())) {
        //case 4: data.add_child(true, 1); // same path value
        //default:
    //}

    return attrib;
}

//
bytespan_t Cluster::makeReportDataMessage(Message const& request) {
    auto resp   = tlvcpp::tlv_tree_node{};
    resp.data() = tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0};

    //
    auto& attrib = resp.add_child(tlvcpp::control_t{2, tlvcpp::e_type::STRUCTURE, 1}, 1);
    auto l0 = request.decodedPayload.TLV.children();
    auto pt = l0.begin();
    for (auto& path : pt->children()) {
        attrib.add_child(makeByPath(path));
    }

    //
    Message outMsg = session.makeMessage(request, 0x05, resp);
    return MessageCodec::encodeMessage(outMsg);
}

#include "../IM.hpp"

//
enum ClusterID: uint16_t {
    BasicInformation = 0x28,
    GeneralCommissioningCluster = 0x30,
    NetworkCommissioningCluster = 0x31,
    TimeSynchronizationCluster = 0x38
};

//
enum GeneralCommissioningClusterEnum: uint8_t {
    Breadcrumb = 0x00,
    BasicCommissioningInfo = 0x01,
    RegulatoryConfig = 0x02,
    LocationCapability = 0x03,
    SupportsConcurrentConnection = 0x04
};

//
enum BasicInformationEnum: uint16_t {
    DataModelRevision = 0x00,
    VendorName = 0x01,
    VendorID = 0x02,
    ProductName = 0x03,
    ProductID = 0x04,
};


//
tlvcpp::tlv_tree_node Cluster::makeByPath(tlvcpp::tlv_tree_node const& path) {
    auto attrib  = tlvcpp::tlv_tree_node(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 0});
    auto& status = attrib.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 1}, 0);
    auto& data   = attrib.add_child(tlvcpp::control_t{1, tlvcpp::e_type::STRUCTURE, 1}, 1);

    // use same path structure (but with redirection)
    { auto redir = path; redir.data() = tlvcpp::tlv(path.data(), 1); data.add_child(redir); };

    // General Commissioning Cluster
    if (uint32_t(path.find(3)->data()) == ClusterID::GeneralCommissioningCluster) { // (General Commissioning Cluster)
        switch (uint32_t(path.find(4)->data())) {
            case GeneralCommissioningClusterEnum::Breadcrumb:                   data.add_child(uint64_t(0), 2); break; // Breadcrumb
            case GeneralCommissioningClusterEnum::BasicCommissioningInfo:       data.add_child(uint16_t(0), 2); break; // BasicCommissioningInfo
            case GeneralCommissioningClusterEnum::RegulatoryConfig:             data.add_child(uint8_t( 0), 2); break; // RegulatoryConfig
            case GeneralCommissioningClusterEnum::LocationCapability:           data.add_child(uint8_t( 0), 2); break; // LocationCapability
            case GeneralCommissioningClusterEnum::SupportsConcurrentConnection: data.add_child(true       , 2); break; // SupportsConcurrentConnection
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
    return session.encodeMessage(outMsg);
}

#ifndef F7AB558A_0FFF_42EB_A8A1_D86920D78416
#define F7AB558A_0FFF_42EB_A8A1_D86920D78416
#include "./core/session.hpp"

//
enum IMProtocolCode : uint32_t {
    ReadRequestMessage = 0x02,
    ReportDataMessage = 0x05
    //0x05
};

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
class Cluster {
public:
    inline void init() { session = Session(); }
    inline Cluster() { init(); }
    inline Cluster(Session const& session) : session(session) {  }

    //
    tlvcpp::tlv_tree_node makeByPath(tlvcpp::tlv_tree_node const& path);
    bytespan_t makeReportDataMessage(Message const& request);
    bytespan_t makeAckMessage(Message const& request);

    //
    SessionKeys& makeSessionKeys();
    Session& getSession() { return session; }
    Session const& getSession() const { return session; }

    //
    inline int const& status() const { return status_; }
    inline bytespan_t handleMessage(Message const& message) {
        switch (message.decodedPayload.header.protocolCode) {
            case IMProtocolCode::ReadRequestMessage: return makeReportDataMessage(message); // when PASE final phase (confirm status)
            //case 0x04: //return makeReportStatus(message, 0);
            default: return {};
        }
        return {};
    }

    //
private: Session session = {};
int status_ = -1;
};

//
#endif

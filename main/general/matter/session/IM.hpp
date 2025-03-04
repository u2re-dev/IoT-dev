#ifndef F7AB558A_0FFF_42EB_A8A1_D86920D78416
#define F7AB558A_0FFF_42EB_A8A1_D86920D78416
#include "./core/session.hpp"

//
class Cluster {
public:
    inline void init() { session = Session(); }
    inline Cluster() { init(); }
    inline Cluster(Session const& session) : session(session) {  }

    //
    tlvcpp::tlv_tree_node makeByPath(tlvcpp::tlv_tree_node const& path);
    bytespan_t makeReportDataMessage(Message const& request);

    //
    SessionKeys& makeSessionKeys();
    Session& getSession() { return session; }
    Session const& getSession() const { return session; }

    //
    inline int const& status() const { return status_; }
    inline bytespan_t handleMessage(Message const& message) {
        switch (message.decodedPayload.header.protocolCode) {
            case 0x02: return makeReportDataMessage(message); // when PASE final phase (confirm status)
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

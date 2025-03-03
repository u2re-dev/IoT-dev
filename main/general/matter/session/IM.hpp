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
    bytespan_t makeReportStatus(Message const& request, uint16_t const& status = 0);

    //
    SessionKeys& makeSessionKeys();
    Session& getSession() { return session; }
    Session const& getSession() const { return session; }

    //
private: Session session = {};
};

//
#endif

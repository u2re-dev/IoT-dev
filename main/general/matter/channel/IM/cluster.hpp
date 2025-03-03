#ifndef F7AB558A_0FFF_42EB_A8A1_D86920D78416
#define F7AB558A_0FFF_42EB_A8A1_D86920D78416
#include "../channel.hpp"


//
class Cluster {
public:
    inline void init() { channel = Channel(); }
    inline Cluster() { init(); }
    inline Cluster(Channel const& channel) : channel(channel) {  }

    //
    tlvcpp::tlv_tree_node makeByPath(tlvcpp::tlv_tree_node const& path);
    bytespan_t makeReportDataMessage(Message const& request);
    bytespan_t makeReportStatus(Message const& request, uint16_t const& status = 0);

    //
    SessionKeys& makeSessionKeys();

    //
    Channel& getChannel() { return channel; }
    Channel const& getChannel() const { return channel; }

    //
    inline uint8_t handlePayload(Payload const& payload) {
        switch (payload.header.protocolCode) {
            //case 0x20: return handlePASERequest(payload);
            //case 0x22: return handlePAKE1(payload);
            //case 0x24: return handlePAKE3(payload);
            default: return 0;
        }
        return payload.header.protocolCode;
    }

    //
private: //
    Channel channel = {};
};

//
#endif

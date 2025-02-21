//#include <arpa/inet.h>
#include "./mdns.hpp"
#include "./udp.hpp"
#include "./PASE.hpp"

//
#define ENABLE_MATTER_TEST //
//#define ENABLE_SPAKE2P_TEST

//
#ifdef ENABLE_MATTER_TEST
int main() {
    MDNS comission = {};
    UDP socket = {};
    PASE pase = {};
    pase.init();

    //
    comission.init();
    comission.service();
    comission.commit();

    //
    socket.init();
    while (true) {
        auto resp = socket.handleRequest();
        if (resp) {
            auto msg  = pase.decodeMessage(resp);
            auto type = pase.handlePayload(msg.decodedPayload);

            //
            if (type) { socket.sendResponse(pase.makeAckMessage(msg)); };
            switch (type) {
                case 0x24: socket.sendResponse(pase.makeReportStatus(msg)); break;
                case 0x20: socket.sendResponse(pase.makePASEResponse(msg)); break;
                case 0x22: socket.sendResponse(pase.makePAKE2(msg)); break;
                default: break;
            }
        }
    }

    //
    return 0;
}
#endif

#ifdef ENABLE_SPAKE2P_TEST
//
int main () {
    
    auto salt = mpi_t().random();

    // from client and server
    auto client = Spake2p( PbkdfParameters{ 10000, salt}, 00 );
    auto server = Spake2p( PbkdfParameters{ 10000, salt}, 00 );

    // compute X from client and Y from server
    auto cX = client.computeX();
    auto sY = server.computeY();

    // generate same secrets
    const auto serverVer = server.computeSecretAndVerifiersFromX( cX );
    const auto clientVer = client.computeSecretAndVerifiersFromY( sY );
}
#endif

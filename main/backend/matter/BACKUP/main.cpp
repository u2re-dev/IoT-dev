//#include <arpa/inet.h>
#include "./mdns.hpp"
#include "./udp.hpp"
#include "./matter.hpp"

//
#define ENABLE_MATTER_TEST //
//#define ENABLE_SPAKE2P_TEST

//
#ifdef ENABLE_MATTER_TEST
int main() {
    MDNS comission = {};
    UDP socket = {};
    Matter matter = {};
    matter.init();
    
    //
    comission.init();
    comission.service();
    comission.commit();
    
    //
    socket.init();
    while (true) {
        auto resp = socket.handleRequest();
        if (resp) {
            auto msg = matter.decodeUnencryptedMessage(resp);
            auto req = matter.handlePAKERequest(msg.decodedPayload);
            auto Xp  = matter.handlePAKE1(msg.decodedPayload);

            //
            if (req.has_value()) {
                socket.sendResponse(matter.makeAckMessage(msg));
                socket.sendResponse(matter.makePAKEResponse(msg));
            }

            //
            if (Xp.has_value()) {
                socket.sendResponse(matter.makeAckMessage(msg));
                socket.sendResponse(matter.makePAKE2(msg));
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

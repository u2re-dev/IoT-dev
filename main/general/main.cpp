//#include <arpa/inet.h>
#include "./mdns.hpp"
#include "./udp.hpp"

//
#include <matter/session/PASE.hpp>

//
#define ENABLE_MATTER_TEST //
//#define ENABLE_SPAKE2P_TEST

//
#ifdef ENABLE_MATTER_TEST

//
PASE handlePASE(UDP& socket) {
    PASE pase = {};
    Session& session = pase.getSession();

    //
    while (true) {
        auto resp = socket.handleRequest();
        if (resp) {
            auto msg  = session.decodeMessage(resp);
            auto type = pase.handlePayload(msg.decodedPayload);
            if (type) {
                socket.sendResponse(session.makeAckMessage(msg));
                socket.sendResponse(pase.makeResponse(msg));
            };
            if (pase.status() == 0) return pase;
        }
    }

    //
    return pase;
}

//
int main() {
    MDNS comission = {};

    //
    comission.init();
    comission.service();
    comission.commit();

    //
    UDP socket = {};
    socket.init();

    //
    PASE pase = handlePASE(socket);


    //
    return 0;
}
#endif

//
#ifdef ENABLE_SPAKE2P_TEST
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

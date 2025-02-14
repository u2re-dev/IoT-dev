#include "ecdh.hpp"

//
#ifdef ENABLE_ECDH
#include <iostream>

//
namespace ECDH {
    std::pair<eccp_t, bigint_t> generateKeyPair() {
        const CurveParameters& curveParams = eccp_t::getCurveParameters();
        bigint_t privateKey = bigint_t::generateRandom(256) % (curveParams.n - (static_cast<unsigned long int>(1))) + (static_cast<unsigned long int>(1));
        eccp_t publicKey;
        publicKey= eccp_t(curveParams.Gx, curveParams.Gy);
        publicKey = publicKey * privateKey;  // Scalar multiplication
        return {publicKey, privateKey};
    }

    eccp_t computeSharedSecret(const bigint_t& privateKey, const eccp_t& otherPublicKey) {
        return otherPublicKey * privateKey;
    }
}
#endif

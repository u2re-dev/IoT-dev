#include "ecdh.hpp"
#include "ecc.hpp"
#include <iostream>

namespace ECDH {

std::pair<Ecc_Point, BigInt> generateKeyPair() {
    const CurveParameters& curveParams = Ecc_Point::GetCurveParameters();
    BigInt privateKey = BigInt::generateRandom(256) % (curveParams.n - (static_cast<unsigned long int>(1))) + (static_cast<unsigned long int>(1));
    Ecc_Point publicKey;
    publicKey= Ecc_Point(curveParams.Gx, curveParams.Gy);
    publicKey = publicKey * privateKey;  // Scalar multiplication
    return {publicKey, privateKey};
}

Ecc_Point computeSharedSecret(const BigInt& privateKey, const Ecc_Point& otherPublicKey) {    
    return otherPublicKey * privateKey;
}

}
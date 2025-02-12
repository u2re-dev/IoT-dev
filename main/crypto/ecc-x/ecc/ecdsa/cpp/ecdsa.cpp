#include "ecdsa.hpp"
#include"ecc.hpp"
#include "bigint.hpp"
#include <iomanip>
#include <sstream>


std::string bytesToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return oss.str();
}

std::pair<BigInt, BigInt> ECDSA::sign(const BigInt& privateKey, const std::vector<uint8_t>& message) {
    const CurveParameters& curveParams = Ecc_Point::GetCurveParameters();
    // 1. Generate a random number k.
    BigInt k= BigInt::generateRandom(256) % (curveParams.n - (static_cast<unsigned long int>(1))) + (static_cast<unsigned long int>(1));
    
    // 2. Compute the point (x, y) = k * G, where G is the generator point.
    Ecc_Point G=Ecc_Point(curveParams.Gx, curveParams.Gy);
    G= G*k;

    // 3. Compute r = x mod n (where n is the order of the curve).

    BigInt r= G.getX() % curveParams.n;
    
    // 4. Compute s = k^(-1) * (hash + r * privateKey) mod n.
    BLAKE2b blake2b(64);
    auto hash = blake2b.hash(std::vector<uint8_t>(message.begin(), message.end()));

    std::string hashStr= bytesToHex(hash);
    BigInt hsh(hashStr, 16);

    BigInt s= (hsh + r * privateKey) * k.modInverse(curveParams.n);
    s = s % curveParams.n; 

    // 5. The signature is the pair (r, s).
    return {r,s};
}

bool ECDSA::verify(const Ecc_Point& publicKey, const std::vector<uint8_t>& message, const std::pair<BigInt, BigInt>& signature) {
    const CurveParameters& curveParams = Ecc_Point::GetCurveParameters();

    // 1. Extract r and s from the signature.
    BigInt r = signature.first;
    BigInt s = signature.second;

    // 2. Compute w = s^(-1) mod n.
    BigInt w = s.modInverse(curveParams.n);

    // Hash the message
    BLAKE2b blake2b(64);
    auto hash = blake2b.hash(std::vector<uint8_t>(message.begin(), message.end()));
    std::string hashStr = bytesToHex(hash);
    BigInt hashedMessage(hashStr, 16);

    // 3. Compute u1 = hash * w mod n and u2 = r * w mod n.
    BigInt u1 = (hashedMessage * w) % curveParams.n;
    BigInt u2 = (r * w) % curveParams.n;

    // 4. Compute the point (x, y) = u1 * G + u2 * publicKey.
    Ecc_Point G = Ecc_Point(curveParams.Gx, curveParams.Gy);
    Ecc_Point point = (G * u1) + (publicKey * u2);

    // 5. The signature is valid if x mod n equals r.
    return (point.getX() % curveParams.n == r);
}

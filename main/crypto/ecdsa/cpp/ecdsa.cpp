#include "ecdsa.hpp"

//
#ifdef ENABLE_ECDSA
#include <iomanip>
#include <sstream>

//
std::pair<bigint_t, bigint_t> ECDSA::sign(const bigint_t& privateKey, const std::vector<uint8_t>& message) {
    const CurveParameters& curveParams = eccp_t::getCurveParameters();
    // 1. Generate a random number k.
    bigint_t k= bigint_t::generateRandom(256) % (curveParams.n - (static_cast<unsigned long int>(1))) + (static_cast<unsigned long int>(1));

    // 2. Compute the point (x, y) = k * G, where G is the generator point.
    eccp_t G=eccp_t(curveParams.Gx, curveParams.Gy);
    G= G*k;

    // 3. Compute r = x mod n (where n is the order of the curve).

    bigint_t r= G.getX() % curveParams.n;

    // 4. Compute s = k^(-1) * (hash + r * privateKey) mod n.
    BLAKE2b blake2b(64);
    auto hash = blake2b.hash(std::vector<uint8_t>(message.begin(), message.end()));

    std::string hashStr= b2h(hash);
    bigint_t hsh(hashStr, 16);

    bigint_t s= (hsh + r * privateKey) * k.modInverse(curveParams.n);
    s = s % curveParams.n;

    // 5. The signature is the pair (r, s).
    return {r,s};
}

//
bool ECDSA::verify(const eccp_t& publicKey, const std::vector<uint8_t>& message, const std::pair<bigint_t, bigint_t>& signature) {
    const CurveParameters& curveParams = eccp_t::getCurveParameters();

    // 1. Extract r and s from the signature.
    bigint_t r = signature.first;
    bigint_t s = signature.second;

    // 2. Compute w = s^(-1) mod n.
    bigint_t w = s.modInverse(curveParams.n);

    // Hash the message
    BLAKE2b blake2b(64);
    auto hash = blake2b.hash(std::vector<uint8_t>(message.begin(), message.end()));
    std::string hashStr = b2h(hash);
    bigint_t hashedMessage(hashStr, 16);

    // 3. Compute u1 = hash * w mod n and u2 = r * w mod n.
    bigint_t u1 = (hashedMessage * w) % curveParams.n;
    bigint_t u2 = (r * w) % curveParams.n;

    // 4. Compute the point (x, y) = u1 * G + u2 * publicKey.
    eccp_t G = eccp_t(curveParams.Gx, curveParams.Gy);
    eccp_t point = (G * u1) + (publicKey * u2);

    // 5. The signature is valid if x mod n equals r.
    return (point.getX() % curveParams.n == r);
}
#endif

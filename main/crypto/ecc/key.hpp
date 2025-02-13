#pragma once
#ifdef ENABLE_ECC_MISC
inline BigInt toPriv(const std::string &priv) {
    BigInt d = std::stoull(priv, nullptr, 16);
    if(d > 0 && d < ECCPoint::N) return d;
    throw std::runtime_error("private key invalid");
}

//
inline Bytes getPublicKey(const std::string &priv, bool isCompressed = true) {
    BigInt d = toPriv(priv);
    ECCPoint Q = ECCPoint::BASE.multiply(d);
    return Q.toRawBytes(isCompressed);
}
#endif

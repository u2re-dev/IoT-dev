#pragma once
#ifdef ENABLE_ECC_MISC
inline bigint_t toPriv(const std::string &priv) {
    bigint_t d = std::stoull(priv, nullptr, 16);
    if(d > 0 && d < eccp_t::N) return d;
    throw std::runtime_error("private key invalid");
}

//
inline bytes_t getPublicKey(const std::string &priv, bool isCompressed = true) {
    bigint_t d = toPriv(priv);
    eccp_t Q = eccp_t::BASE.multiply(d);
    return Q.toRawbytes_t(isCompressed);
}
#endif

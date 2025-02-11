#include "../openssl.hpp"
#include "../spake2p.hpp"

//
#ifdef USE_OPENSSL
static EC_POINT* Spake2p::computeK(EC_GROUP* group, const EC_POINT* P) {
    EC_POINT* K = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();

    //
    EC_POINT* temp1 = EC_POINT_new(group);
    if (!EC_POINT_mul(group, temp1, random, nullptr, nullptr, ctx)) { std::cerr << "EC_POINT_mul(G, r) failed\n"; }

    //
    EC_POINT* temp2 = EC_POINT_new(group);
    if (!EC_POINT_mul(group, temp2, nullptr, P, w0, ctx)) { std::cerr << "EC_POINT_mul(M, w0) failed\n"; }
    if (!EC_POINT_add(group, K, temp1, temp2, ctx)) { std::cerr << "EC_POINT_add failed\n"; }

    //
    EC_POINT_free(temp1);
    EC_POINT_free(temp2);
    BN_CTX_free(ctx);
    return X;
}

//
#ifdef ENABLE_PAKE_CLIENT
static EC_POINT* Spake2p::computeL(EC_GROUP* group, const BIGNUM* w1) {
    BN_CTX* ctx = BN_CTX_new();
    if (!ctx) return nullptr;

    //
    EC_POINT* L = EC_POINT_new(group);
    if (!L) { BN_CTX_free(ctx); return nullptr; }

    //
    if (!EC_POINT_mul(group, L, w1, nullptr, nullptr, ctx)) {
        EC_POINT_free(L);
        BN_CTX_free(ctx);
        return nullptr;
    }

    //
    BN_CTX_free(ctx);
    return L;
}
#endif

//
#ifdef ENABLE_PAKE_CLIENT
EC_POINT* Spake2p::computeX() { return Spake2p::compute(M); }
#endif

//
#ifdef ENABLE_PAKE_SERVER
EC_POINT* Spake2p::computeY() { return Spake2p::compute(N); }
#endif
#endif

#pragma once
#include "./openssl.hpp"

//
class Spake2p {
private:
    const char* M_hex = "02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f";
    const char* N_hex = "03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49";

public:
    std::vector<unsigned char> context;

    //
#ifdef USE_OPENSSL
    BIGNUM* random, w0;
    EC_GROUP* group;
    EC_POINT* M, N;

    //
    Spake2p(const std::vector<unsigned char>& ctx, BIGNUM* r, BIGNUM* w0_val, EC_GROUP* grp, EC_POINT* M_point, EC_POINT* N_point) : context(ctx), random(r), w0(w0_val), group(grp)
    {
        M = EC_POINT_dup(M_point, group);
        N = EC_POINT_dup(N_point, group);
    }

    ~Spake2p() {
        if (random) BN_free(random);
        if (w0) BN_free(w0);
        if (M) EC_POINT_free(M);
        if (N) EC_POINT_free(N);
        if (group) EC_GROUP_free(group);
    }

    //
#ifdef ENABLE_PAKE_SERVER
    static int computeW0W1(uint32_t pin, const std::vector<unsigned char>& salt, int iterations, BIGNUM** w0, BIGNUM** w1);
    static Spake2p* create(const std::vector<unsigned char>& ctx, BIGNUM* w0_val);
#endif

    //
    static EC_POINT* computeL(EC_GROUP* group, const BIGNUM* w1);
    static EC_POINT* computeK(EC_GROUP* group, const EC_POINT* P);

    //
#ifdef ENABLE_PAKE_CLIENT
    EC_POINT* computeX();
    bool computeSecretAndVerifiersFromY(const BIGNUM* w1, const EC_POINT* X, const EC_POINT* Y, std::vector<unsigned char>& Ke, std::vector<unsigned char>& hAY, std::vector<unsigned char>& hBX);
#endif

    //
#ifdef ENABLE_PAKE_SERVER
    EC_POINT* computeY();
    bool computeSecretAndVerifiersFromX(EC_POINT* L, const EC_POINT* X, const EC_POINT* Y, std::vector<unsigned char>& Ke, std::vector<unsigned char>& hAY, std::vector<unsigned char>& hBX);
#endif
#endif

};

#include "../openssl.hpp"
#include "../spake2p.hpp"

// no sense in ESP32-device
#ifdef ENABLE_PAKE_SERVER
static Spake2p* Spake2p::create(const std::vector<unsigned char>& ctx, BIGNUM* w0_val) {
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (!group) { std::cerr << "EC_GROUP_new_by_curve_name failed\n"; return nullptr; }

    //
    EC_POINT* M_point = EC_POINT_new(group);
    EC_POINT* N_point = EC_POINT_new(group);
    if (!EC_POINT_hex2point(group, M_hex, M_point, nullptr) ||
        !EC_POINT_hex2point(group, N_hex, N_point, nullptr)) {
        std::cerr << "Ошибка создания точек M или N\n";
        EC_POINT_free(M_point); EC_POINT_free(N_point);
        EC_GROUP_free(group);
        return nullptr;
    }

    //
    BN_CTX* bn_ctx = BN_CTX_new();
    BIGNUM* order = BN_new(); EC_GROUP_get_order(group, order, bn_ctx);

    //
    BIGNUM* r = BN_new();
    if (!BN_rand_range(r, order)) {
        std::cerr << "BN_rand_range failed\n";
        BN_free(order); BN_free(r); BN_CTX_free(bn_ctx);
        EC_POINT_free(M_point); EC_POINT_free(N_point); EC_GROUP_free(group);
        return nullptr;
    }

    //
    BN_free(order);
    BN_CTX_free(bn_ctx);

    //
    Spake2p* sp = new Spake2p(ctx, r, w0_val, group, M_point, N_point);
    EC_POINT_free(M_point);
    EC_POINT_free(N_point);
    return sp;
}
#endif

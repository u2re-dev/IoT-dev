#include <iostream>
#include <vector>
#include <cstring>

//
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

//
class Spake2p {
private:
    //
    const char* M_hex = "02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f";
    const char* N_hex = "03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49";

public:
    std::vector<unsigned char> context;
    BIGNUM* random;
    BIGNUM* w0;

    //
    EC_GROUP* group;
    EC_POINT* M;
    EC_POINT* N;

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
    static Spake2p* create(const std::vector<unsigned char>& ctx, BIGNUM* w0_val) {
        EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
        if (!group) {
            std::cerr << "EC_GROUP_new_by_curve_name failed\n";
            return nullptr;
        }
        const char* M_hex = "02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f";
        const char* N_hex = "03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49";
        EC_POINT* M_point = EC_POINT_new(group);
        EC_POINT* N_point = EC_POINT_new(group);
        if (!EC_POINT_hex2point(group, M_hex, M_point, nullptr) ||
            !EC_POINT_hex2point(group, N_hex, N_point, nullptr)) {
            std::cerr << "Ошибка создания точек M или N\n";
            EC_POINT_free(M_point); EC_POINT_free(N_point);
            EC_GROUP_free(group);
            return nullptr;
        }
        BN_CTX* bn_ctx = BN_CTX_new();
        BIGNUM* order = BN_new();
        EC_GROUP_get_order(group, order, bn_ctx);
        BIGNUM* r = BN_new();
        if (!BN_rand_range(r, order)) {
            std::cerr << "BN_rand_range failed\n";
            BN_free(order); BN_free(r); BN_CTX_free(bn_ctx);
            EC_POINT_free(M_point); EC_POINT_free(N_point); EC_GROUP_free(group);
            return nullptr;
        }
        BN_free(order);
        BN_CTX_free(bn_ctx);

        Spake2p* sp = new Spake2p(ctx, r, w0_val, group, M_point, N_point);
        EC_POINT_free(M_point);
        EC_POINT_free(N_point);
        return sp;
    }

    //
    EC_POINT* computeL(const BIGNUM* w1) {
        BN_CTX* ctx = BN_CTX_new();
        if (!ctx) return nullptr;
        EC_POINT* L = EC_POINT_new(group);
        if (!L) { BN_CTX_free(ctx); return nullptr; }
        // Вычисляем L = G * w1 (то есть умножаем генератор группы на w1)
        if (!EC_POINT_mul(group, L, w1, nullptr, nullptr, ctx)) {
            EC_POINT_free(L);
            BN_CTX_free(ctx);
            return nullptr;
        }
        BN_CTX_free(ctx);
        return L;
    }

    //
    EC_POINT* computeX() {
        EC_POINT* X = EC_POINT_new(group);
        BN_CTX* ctx = BN_CTX_new();
        EC_POINT* temp1 = EC_POINT_new(group);
        if (!EC_POINT_mul(group, temp1, random, nullptr, nullptr, ctx)) {
            std::cerr << "EC_POINT_mul(G, r) failed\n";
        }
        EC_POINT* temp2 = EC_POINT_new(group);
        if (!EC_POINT_mul(group, temp2, nullptr, M, w0, ctx)) {
            std::cerr << "EC_POINT_mul(M, w0) failed\n";
        }
        if (!EC_POINT_add(group, X, temp1, temp2, ctx)) { std::cerr << "EC_POINT_add failed\n"; };
        EC_POINT_free(temp1);
        EC_POINT_free(temp2);
        BN_CTX_free(ctx);
        return X;
    }

    //
    EC_POINT* computeY() {
        EC_POINT* Y = EC_POINT_new(group);
        BN_CTX* ctx = BN_CTX_new();
        EC_POINT* temp1 = EC_POINT_new(group);
        if (!EC_POINT_mul(group, temp1, random, nullptr, nullptr, ctx)) {
            std::cerr << "EC_POINT_mul(G, r) failed\n";
        }
        EC_POINT* temp2 = EC_POINT_new(group);
        if (!EC_POINT_mul(group, temp2, nullptr, N, w0, ctx)) {
            std::cerr << "EC_POINT_mul(N, w0) failed\n";
        }
        if (!EC_POINT_add(group, Y, temp1, temp2, ctx))
            std::cerr << "EC_POINT_add failed\n";
        EC_POINT_free(temp1);
        EC_POINT_free(temp2);
        BN_CTX_free(ctx);
        return Y;
    }

    //
    bool computeSecretAndVerifiersFromY(const BIGNUM* w1, const EC_POINT* X, const EC_POINT* Y,
    std::vector<unsigned char>& Ke, std::vector<unsigned char>& hAY, std::vector<unsigned char>& hBX)
    {
        BN_CTX* ctx = BN_CTX_new();
        if (!ctx) return false;

        //
        EC_POINT* temp = EC_POINT_new(group);
        if (!EC_POINT_mul(group, temp, nullptr, N, w0, ctx)) {
            std::cerr << "EC_POINT_mul(N, w0) failed\n";
            BN_CTX_free(ctx); return false;
        }

        //
        if (!EC_POINT_invert(group, temp, ctx)) {
            std::cerr << "EC_POINT_invert failed\n";
            EC_POINT_free(temp); BN_CTX_free(ctx); return false;
        }

        EC_POINT* Yprime = EC_POINT_new(group);
        if (!EC_POINT_add(group, Yprime, Y, temp, ctx)) {
            std::cerr << "EC_POINT_add (Y - N*w0) failed\n";
            EC_POINT_free(temp); EC_POINT_free(Yprime); BN_CTX_free(ctx); return false;
        }
        EC_POINT_free(temp);

        //
        EC_POINT* Z = EC_POINT_new(group);
        if (!EC_POINT_mul(group, Z, nullptr, Yprime, random, ctx)) {
            std::cerr << "EC_POINT_mul (Yprime*r) failed\n";
            EC_POINT_free(Yprime); BN_CTX_free(ctx); return false;
        }

        //
        EC_POINT* V = EC_POINT_new(group);
        if (!EC_POINT_mul(group, V, nullptr, Yprime, w1, ctx)) {
            std::cerr << "EC_POINT_mul (Yprime*w1) failed\n";
            EC_POINT_free(Yprime); EC_POINT_free(Z); BN_CTX_free(ctx); return false;
        }

        //
        unsigned char bufX[100] = {0}, bufY[100] = {0}, bufZ[100] = {0}, bufV[100] = {0};
        size_t lenX = EC_POINT_point2oct(group, X, POINT_CONVERSION_UNCOMPRESSED, bufX, sizeof(bufX), ctx);
        size_t lenY = EC_POINT_point2oct(group, Y, POINT_CONVERSION_UNCOMPRESSED, bufY, sizeof(bufY), ctx);
        size_t lenZ = EC_POINT_point2oct(group, Z, POINT_CONVERSION_UNCOMPRESSED, bufZ, sizeof(bufZ), ctx);
        size_t lenV = EC_POINT_point2oct(group, V, POINT_CONVERSION_UNCOMPRESSED, bufV, sizeof(bufV), ctx);

        //
        int w0_len = BN_num_bytes(w0);
        std::vector<unsigned char> w0_bytes(w0_len);
        BN_bn2bin(w0, w0_bytes.data());

        //
        std::vector<unsigned char> transcript;
        transcript.insert(transcript.end(), context.begin(), context.end());
        transcript.insert(transcript.end(), bufX, bufX + lenX);
        transcript.insert(transcript.end(), bufY, bufY + lenY);
        transcript.insert(transcript.end(), bufZ, bufZ + lenZ);
        transcript.insert(transcript.end(), bufV, bufV + lenV);
        transcript.insert(transcript.end(), w0_bytes.begin(), w0_bytes.end());

        //
        unsigned char hash[32] = {0};
        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        if (!mdctx) { EC_POINT_free(Yprime); EC_POINT_free(Z); EC_POINT_free(V); BN_CTX_free(ctx); return false; }
        if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) { EVP_MD_CTX_free(mdctx); return false; }
        if (EVP_DigestUpdate(mdctx, transcript.data(), transcript.size()) != 1) { EVP_MD_CTX_free(mdctx); return false; }
        unsigned int hash_len = 0;
        if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) { EVP_MD_CTX_free(mdctx); return false; }
        EVP_MD_CTX_free(mdctx);

        //
        Ke.assign(hash+16, hash+32);
        hAY.assign(hash, hash + 16);
        hBX.assign(hash, hash + 16);

        //
        EC_POINT_free(Yprime);
        EC_POINT_free(Z);
        EC_POINT_free(V);
        BN_CTX_free(ctx);
        return true;
    }

    //
    bool computeSecretAndVerifiersFromX(EC_POINT* L, const EC_POINT* X, const EC_POINT* Y,
    std::vector<unsigned char>& Ke, std::vector<unsigned char>& hAY, std::vector<unsigned char>& hBX)
    {
        BN_CTX* ctx = BN_CTX_new();
        if (!ctx) return false;

        //
        EC_POINT* gx = EC_POINT_new(group);
        if (!EC_POINT_copy(gx, X)) { BN_CTX_free(ctx); return false; }
        if (!EC_POINT_sub(group, gx, gx, M, ctx)) {
            std::cerr << "EC_POINT_sub failed\n";
            EC_POINT_free(gx); BN_CTX_free(ctx); return false;
        }

        //
        EC_POINT* Z = EC_POINT_new(group);
        if (!EC_POINT_mul(group, Z, nullptr, gx, random, ctx)) {
            std::cerr << "EC_POINT_mul Z failed\n";
            EC_POINT_free(gx); BN_CTX_free(ctx); return false;
        }

        //
        EC_POINT* V = EC_POINT_new(group);
        if (!EC_POINT_mul(group, V, nullptr, L, random, ctx)) {
            std::cerr << "EC_POINT_mul V failed\n";
            EC_POINT_free(gx); EC_POINT_free(Z); BN_CTX_free(ctx); return false;
        }

        //
        unsigned char bufX[100] = {0}, bufY[100] = {0}, bufZ[100] = {0}, bufV[100] = {0};
        size_t lenX = EC_POINT_point2oct(group, X, POINT_CONVERSION_UNCOMPRESSED, bufX, sizeof(bufX), ctx);
        size_t lenY = EC_POINT_point2oct(group, Y, POINT_CONVERSION_UNCOMPRESSED, bufY, sizeof(bufY), ctx);
        size_t lenZ = EC_POINT_point2oct(group, Z, POINT_CONVERSION_UNCOMPRESSED, bufZ, sizeof(bufZ), ctx);
        size_t lenV = EC_POINT_point2oct(group, V, POINT_CONVERSION_UNCOMPRESSED, bufV, sizeof(bufV), ctx);

        //
        int w0_len = BN_num_bytes(w0);
        std::vector<unsigned char> w0_bytes(w0_len);
        BN_bn2bin(w0, w0_bytes.data());

        //
        std::vector<unsigned char> transcript;
        transcript.insert(transcript.end(), context.begin(), context.end());
        transcript.insert(transcript.end(), bufX, bufX + lenX);
        transcript.insert(transcript.end(), bufY, bufY + lenY);
        transcript.insert(transcript.end(), bufZ, bufZ + lenZ);
        transcript.insert(transcript.end(), bufV, bufV + lenV);
        transcript.insert(transcript.end(), w0_bytes.begin(), w0_bytes.end());

        //
        unsigned char hash[32] = {0};
        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        if (!mdctx) { EC_POINT_free(gx); EC_POINT_free(Z); EC_POINT_free(V); BN_CTX_free(ctx); return false; }
        if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) { EVP_MD_CTX_free(mdctx); return false; }
        if (EVP_DigestUpdate(mdctx, transcript.data(), transcript.size()) != 1){ EVP_MD_CTX_free(mdctx); return false; }
        unsigned int hash_len = 0;
        if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1){ EVP_MD_CTX_free(mdctx); return false; }
        EVP_MD_CTX_free(mdctx);

        //
        hAY.assign(hash, hash + 16);
        Ke.assign(hash+16, hash+32);
        hBX.assign(hash, hash + 16);

        //
        EC_POINT_free(gx);
        EC_POINT_free(Z);
        EC_POINT_free(V);
        BN_CTX_free(ctx);
        return true;
    }
};

#ifndef D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE
#define D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE

//
#include "../std/types.hpp"
#include "../std/hex.hpp"

//
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <iostream>

//
#include "./mbedtls/ecc_point_compression.h"
#include "./raii/mpi.hpp"
#include "./raii/ecp.hpp"
#include "./raii/misc.hpp"
#include "./crypto.hpp"
#include "spake2p/bigint/intx.hpp"



//
constexpr  uint8_t H_VERSION   = 0x01;
constexpr uint16_t SER_VERSION = 0x0001;

//
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES     = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN           = CRYPTO_W_SIZE_BYTES     * 2;

//
struct SecretAndVerifiers {
    intx::uint128 Ke;
    bigint_t hAY;
    bigint_t hBX;
};

//
struct PbkdfParameters {
    uint16_t iterations;
    bigint_t salt;
};

//
struct W0W1L {
    ecp_t L;
    mpi_t w0;
    mpi_t w1;
    mpi_t random;
    PbkdfParameters params;
};

//
std::string bignumToHex(bigint_t const& I) {
    bytes_t tmp = make_bytes(sizeof(I));
    memcpy(tmp->data(), &I, sizeof(I));
    return hex::b2h(tmp);
}

//
using uncomp_t = bytes_t;
void  print(ecp_t const& P) { std::cout << hex::b2h(P) << std::endl; }
void  printVerifiers(SecretAndVerifiers const& v) {
    std::cout << "==== DEBUG VERIFIERS ====" << std::endl;
    std::cout << bignumToHex(v.Ke) << std::endl;
    std::cout << bignumToHex(v.hAY) << std::endl;
    std::cout << bignumToHex(v.hBX) << std::endl;
    std::cout << "==== DEBUG ENDL ====" << std::endl;
}


//
bytes_t computeContextHash(bytes_t const& req, bytes_t const& res) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts_ret(&ctx, 0);

    //
    const auto name = hex::s2b("CHIP PAKE V1 Commissioning");
    mbedtls_sha256_update_ret(&ctx, name->data(), name->size());
    mbedtls_sha256_update_ret(&ctx, req->data(), req->size());
    mbedtls_sha256_update_ret(&ctx, res->data(), res->size());

    //
    //bigint_t out = bigint_t(0);
    auto out = make_bytes(32);
    checkMbedtlsError(mbedtls_sha256_finish_ret(&ctx, (uint8_t*)out->data()), "Failed to compute Hash");
    return std::move(out);
}


//
class Spake2p {
public:
    ~Spake2p() { mbedtls_ecp_group_free(&group_); }

    //
    Spake2p(const PbkdfParameters& pbkdfParameters, uint32_t const& pin, bytes_t const& context) { 
        mbedtls_ecp_group_init(&group_); 
        mbedtls_ecp_group_load(&group_, MBEDTLS_ECP_DP_SECP256R1);
        //context_ = make_bytes(std::vector<uint8_t>{0x43, 0x48, 0x49, 0x50, 0x20, 0x50, 0x41, 0x4b, 0x45, 0x20, 0x56, 0x31, 0x20, 0x43, 0x6f, 0x6d, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x69, 0x6e, 0x67 });
        base_ = computeW0W1L(group_, pbkdfParameters, pin);
        context_ = context;
    }

    //
    Spake2p(const PbkdfParameters& pbkdfParameters, uint32_t const& pin) { 
        mbedtls_ecp_group_init(&group_); 
        mbedtls_ecp_group_load(&group_, MBEDTLS_ECP_DP_SECP256R1);
        //context_ = make_bytes(std::vector<uint8_t>{0x43, 0x48, 0x49, 0x50, 0x20, 0x50, 0x41, 0x4b, 0x45, 0x20, 0x56, 0x31, 0x20, 0x43, 0x6f, 0x6d, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x69, 0x6e, 0x67 });
        base_ = computeW0W1L(group_, pbkdfParameters, pin);
        context_ = hex::s2b("CHIP PAKE V1 Commissioning");
    }

    //
    Spake2p(Spake2p const& another) : X_(another.X_), group_(another.group_), context_(another.context_), base_(another.base_) {}

    //
    ecp_t parseECP (uint8_t const* stream, size_t length) {
        return ecp_t(group_).loadBytes(stream, length);
    }
    
    //
    bigint_t getRandom() const {
        return base_.random;
    }



    // Device-Side (isn't know initiator random)
    uncomp_t computeY() { Y_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getN(), base_.w0); return std::move(Y_); }
    SecretAndVerifiers computeSecretAndVerifiersFromX( uncomp_t const& bX ) const {
        ecp_t X  = ecp_t(group_, bX);
        ecp_t Lp = base_.L;
        ecp_t Br = X  - ecp_t(group_).getM() * base_.w0; // with foreign factor
        ecp_t Z  = Br * base_.random; // random factor crossed
        ecp_t V  = Lp * base_.random; // always Device-side factor (own)
        return std::move(computeSecretAndVerifiers(X, Y_, Z, V));
    }


    // Initiator //!unused
    uncomp_t computeX() { X_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getM(), base_.w0); return std::move(X_); }
    SecretAndVerifiers computeSecretAndVerifiersFromY( uncomp_t const& bY ) const {
        ecp_t Y  = ecp_t(group_, bY);
        ecp_t Br = Y  - ecp_t(group_).getN() * base_.w0; // with foreign factor
        ecp_t Z  = Br * base_.random; // random factor crossed
        ecp_t V  = Br * base_.w1;     // always Device-side factor (foreign)
        return std::move(computeSecretAndVerifiers(X_, Y, Z, V));
    }


private:

    //
    SecretAndVerifiers computeSecretAndVerifiers( ecp_t X, ecp_t Y, ecp_t Z, ecp_t V) const {
        auto transcript = crypto::hash(computeTranscript(X, Y, Z, V));
        intx::uint128& Ka = *(intx::uint128*)((uint8_t*)&transcript + 0), Ke = *(intx::uint128*)((uint8_t*)&transcript + 16);

        //
        auto info = hex::s2b("ConfirmationKeys");
        bigint_t KcAB = crypto::hkdf(Ka, info);
        intx::uint128& KcA = *(intx::uint128*)((uint8_t*)&KcAB + 0), KcB = *(intx::uint128*)((uint8_t*)&KcAB + 16);

        //
        SecretAndVerifiers result = {};
        result.Ke  = Ke;
        result.hAY = crypto::hmac(KcA, Y);
        result.hBX = crypto::hmac(KcB, X);
        return std::move(result);
    }

    //
    bytes_t computeTranscript(const ecp_t& X, const ecp_t& Y, const ecp_t& Z, const ecp_t& V) const {
        writer_t writer;
        writer.writeUInt64(context_->size()); writer.writeBytes(context_);
        //writer.writeUInt64(32); writer.writeBigInt(ctxHash);
        writer.writeUInt64(0); writer.writeUInt64(0); // identifiers

        // N and M write
        writer.writeUInt64(65); writer.writeBytes(ecp_t(group_).getM());
        writer.writeUInt64(65); writer.writeBytes(ecp_t(group_).getN());

        // X, Y, Z, V points
        writer.writeUInt64(65); writer.writeBytes(X); // pA
        writer.writeUInt64(65); writer.writeBytes(Y); // pB
        writer.writeUInt64(65); writer.writeBytes(Z);
        writer.writeUInt64(65); writer.writeBytes(V);

        // writing w0 and compute hash
        writer.writeUInt64(32); writer.writeBigInt(base_.w0);
        return writer;
    }

    //
    static ecp_t computeLPoint(mbedtls_ecp_group const& group, mpi_t const& w1) {
        return std::move(ecp_t(group, group.G) * w1);
    }

    //
    static W0W1L computeW0W1L(mbedtls_ecp_group const& group, const PbkdfParameters& pbkdfParameters, uint32_t const& pin) {
        

        auto ws = crypto::pbkdf2((uint8_t const*)&pin, 4, pbkdfParameters.salt, pbkdfParameters.iterations, PBKDF2_OUTLEN);
        if (ws->size() < PBKDF2_OUTLEN) { throw std::runtime_error("PBKDF2: not enough length"); }

        //
        auto m0 = make_bytes(ws->begin(), ws->begin() + CRYPTO_W_SIZE_BYTES);
        auto m1 = make_bytes(ws->begin() + CRYPTO_W_SIZE_BYTES, ws->begin() + PBKDF2_OUTLEN);

        //
        //std::reverse(m0->begin(), m0->end());
        //std::reverse(m1->begin(), m1->end());

        //
        auto w0 = mpi_t(m0) % group.N;
        auto w1 = mpi_t(m1) % group.N;

        //
        W0W1L w0w1L = {};
        w0w1L.w0 = w0, w0w1L.w1 = w1;
        w0w1L.L  = computeLPoint(group, w1);
        w0w1L.random = mpi_t().random() % group.P;
        w0w1L.params = pbkdfParameters;
        return std::move(w0w1L);
    }

    //
    union { ecp_t X_, Y_; };
    mbedtls_ecp_group group_;
    bytes_t context_ = {};
    W0W1L base_ = {};
};

#endif

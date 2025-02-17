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
    uint32_t iterations;
     bytes_t salt;
};

//
struct W0W1L {
    bigint_t w0;
    bigint_t w1;
    bigint_t random;
    ecp_t L;
    PbkdfParameters params;
};


//
void print(ecp_t const& P) { std::cout << hex::b2h(P) << std::endl; }

//
using uncomp_t = bytes_t;

//
class Spake2p {
public:
    ~Spake2p() { mbedtls_ecp_group_free(&group_); }
    Spake2p(const bytes_t& context, const W0W1L& base) : context_(context), base_(base) { 
        mbedtls_ecp_group_init(&group_); 
        mbedtls_ecp_group_load(&group_, MBEDTLS_ECP_DP_SECP256R1);
        //context_ = make_bytes();
    }

    //
    static W0W1L computeW0W1L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        auto ws = crypto::pbkdf2(pinToBytes(pin), pbkdfParameters.salt, pbkdfParameters.iterations, PBKDF2_OUTLEN);
        if (ws->size() < PBKDF2_OUTLEN) { throw std::runtime_error("PBKDF2: not enough length"); }

        // TODO: use single group instead of recreating group
        mbedtls_ecp_group group; mbedtls_ecp_group_init(&group);
        checkMbedtlsError(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1), "ECC group load failed");

        //
        W0W1L w0w1L = {};
        w0w1L.w0 = mpi_t(ws->data(), CRYPTO_W_SIZE_BYTES) % group.N;
        w0w1L.w1 = mpi_t(ws->data() + CRYPTO_W_SIZE_BYTES, CRYPTO_W_SIZE_BYTES) % group.N;
        w0w1L.L  = computeLPoint(group, w0w1L.w1);
        w0w1L.random = mpi_t().random();
        w0w1L.params = pbkdfParameters;

        //
        mbedtls_ecp_group_free(&group);
        return std::move(w0w1L);
    }

    // aka. pA
    uncomp_t computeX() { X_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getM(), base_.w0); return X_; }

    // aka. pB
    uncomp_t computeY() { Y_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getN(), base_.w0); return Y_; }


    // to-server (X and L from client), aka. by Y  aka. cA
    SecretAndVerifiers computeSecretAndVerifiersFromX( uncomp_t const& bX, uncomp_t const& L) const {
        ecp_t Lp = ecp_t(group_, L);
        ecp_t X  = ecp_t(group_, bX);
        ecp_t Br = X  - ecp_t(group_).getM() * base_.w0;
        ecp_t Z  = Br * base_.random;
        ecp_t V  = Lp * base_.random;
        return computeSecretAndVerifiers(X, Y_, Z, V);
    }

    // to-client (Y and w1 from server), aka. by X,  aka. cB
    SecretAndVerifiers computeSecretAndVerifiersFromY( uncomp_t const& bY, bigint_t const& w1b) const {
        mpi_t w1 = mpi_t(w1b);
        ecp_t Y  = ecp_t(group_, bY);
        ecp_t Br = Y  - ecp_t(group_).getN() * base_.w0;
        ecp_t Z  = Br * base_.random;
        ecp_t V  = Br * base_.w1;
        return computeSecretAndVerifiers(X_, Y, Z, V);
    }



private:

    //
    SecretAndVerifiers computeSecretAndVerifiers( ecp_t X, ecp_t Y, ecp_t Z, ecp_t V) const {
        print(X); print(Y); print(Z); print(V);

        //
        auto& info = context_;//hex::s2b("ConfirmationKeys");

        //
        bigint_t&& transcript = computeTranscriptHash(X, Y, Z, V);
        intx::uint128& Ka = *(intx::uint128*)((uint8_t*)&transcript + 0), Ke = *(intx::uint128*)((uint8_t*)&transcript + 16);

        //
        bigint_t&& KcAB = crypto::hkdf(&Ka, sizeof(Ka), base_.params.salt, info);
        intx::uint128& KcA = *(intx::uint128*)((uint8_t*)&KcAB + 0), KcB = *(intx::uint128*)((uint8_t*)&KcAB + 16);

        //
        SecretAndVerifiers result = {};
        result.Ke = Ke;
        result.hAY = crypto::hmac(KcA, Y);
        result.hBX = crypto::hmac(KcB, X);
        return std::move(result);
    }

    //
    bigint_t computeTranscriptHash(const ecp_t& X, const ecp_t& Y, const ecp_t& Z, const ecp_t& V) const {
        writer_t writer; writer.writeBytes(context_);
        writer.writeBigint(bigint_t(0));

        // N and M write
        writer.writeBytes(ecp_t(group_).getM());
        writer.writeBytes(ecp_t(group_).getN());

        // X, Y, Z, V points
        writer.writeBytes(X); // pA
        writer.writeBytes(Y); // pB
        writer.writeBytes(Z);
        writer.writeBytes(V);

        // writing w0 and compute hash
        writer.writeBytes(hex::n2b(base_.w0));
        return std::move(crypto::hash(writer));
    }

    //
    static bytes_t pinToBytes(uint32_t pin) {
        bytes_t pinBytes = make_bytes(4);
        memcpy(pinBytes->data(), &pin, sizeof(pin));
        return pinBytes;
    }

    //
    static ecp_t computeLPoint(mbedtls_ecp_group const& group, mpi_t const& w1) {
        return std::move(ecp_t(group, group.G) * w1);
    }


    //
    union { ecp_t X_, Y_; };
    mbedtls_ecp_group group_;
    bytes_t context_ = {};
    W0W1L base_ = {};
};

#endif

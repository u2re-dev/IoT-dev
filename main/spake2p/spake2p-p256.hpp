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
    bigint_t w0;
    bigint_t w1;
    bigint_t random;
    PbkdfParameters params;
};

//
std::string bignumToHex(bigint_t const& I) {
    bytes_t tmp = make_bytes(sizeof(I));
    memcpy(tmp->data(), &I, sizeof(I));
    return hex::b2h(tmp);
}

//
void printVerifiers(SecretAndVerifiers const& v) {
    std::cout << "==== DEBUG VERIFIERS ====" << std::endl;
    std::cout << bignumToHex(v.Ke) << std::endl;
    std::cout << bignumToHex(v.hAY) << std::endl;
    std::cout << bignumToHex(v.hBX) << std::endl;
    std::cout << "==== DEBUG ENDL ====" << std::endl;
}


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
    Spake2p(Spake2p const& another) : X_(another.X_), group_(another.group_), context_(another.context_), base_(another.base_) {}

    //
    static W0W1L computeW0W1L(const PbkdfParameters& pbkdfParameters, uint32_t const& pin) {
        auto ws = crypto::pbkdf2(pinToBytes(pin), pbkdfParameters.salt, pbkdfParameters.iterations, PBKDF2_OUTLEN);
        std::reverse(ws->begin(), ws->end());
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

    //
    ecp_t parseECP (uint8_t const* stream, size_t length) {
        return ecp_t(group_).loadBytes(stream, length);
    }
    


    // Device-Side (isn't know initiator random)
    uncomp_t computeY() { Y_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getN(), base_.w0); return std::move(Y_); }
    SecretAndVerifiers computeSecretAndVerifiersFromX( uncomp_t const& bX ) const {
        ecp_t X  = ecp_t(group_, bX);
        ecp_t Lp  = base_.L;
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




    // use import X/Y value instead of internal (import initiator random, use client L, ...)
    bool verifyXCorrect( uncomp_t const& bX, /*uncomp_t const& L*/ /*bigint_t const& w1b,*/ bigint_t const& rand) const {
        ecp_t X  = ecp_t(group_, bX);
        ecp_t Lp = base_.L;//ecp_t(group_, L);
        ecp_t Y  = Y_;

        //
        ecp_t Br = X - ecp_t(group_).getM() * base_.w0; // import's factor (M)
        ecp_t Or = Y - ecp_t(group_).getN() * base_.w0; // own's factor (N)

        //
        ecp_t CrI = Br * base_.random; // cross with client
        ecp_t crO = Or * rand;         // cross with server
        std::cout << "==== DEBUG COPS ====" << std::endl;
        print(CrI); print(crO);
        std::cout << "==== DEBUG ENDS ====" << std::endl;
        return (CrI == crO);
    }


private:

    //
    SecretAndVerifiers computeSecretAndVerifiers( ecp_t X, ecp_t Y, ecp_t Z, ecp_t V) const {
        std::cout << "==== DEBUG COMPONENTS ====" << std::endl;
        print(X); print(Y); print(Z); print(V);
        std::cout << "==== DEBUG ENDS ====" << std::endl;

        //
        auto info = hex::s2b("ConfirmationKeys");

        //
        bigint_t transcript = computeTranscriptHash(X, Y, Z, V);
        intx::uint128& Ka = *(intx::uint128*)((uint8_t*)&transcript + 0), Ke = *(intx::uint128*)((uint8_t*)&transcript + 16);

        //
        bigint_t&& KcAB = crypto::hkdf(&Ka, info);
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
        writer.writeUInt64(0);

        // N and M write
        writer.writeBytes(ecp_t(group_).getM());
        writer.writeBytes(ecp_t(group_).getN());

        // X, Y, Z, V points
        writer.writeBytes(X); // pA
        writer.writeBytes(Y); // pB
        writer.writeBytes(Z);
        writer.writeBytes(V);

        // writing w0 and compute hash
        writer.writeBigInt(intx::to_big_endian(base_.w0));
        return std::move(crypto::hash(writer));
    }

    //
    static bytes_t pinToBytes(uint16_t const& pin) {
        bytes_t pinBytes = make_bytes(2);
        memcpy(pinBytes->data(), &pin, sizeof(pin));
        std::reverse(pinBytes->begin(), pinBytes->end());
        return pinBytes;
    }

    //
    static bytes_t pinToBytes(uint32_t const& pin) {
        bytes_t pinBytes = make_bytes(4);
        memcpy(pinBytes->data(), &pin, sizeof(pin));
        std::reverse(pinBytes->begin(), pinBytes->end());
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

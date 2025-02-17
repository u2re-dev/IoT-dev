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
    bigint_t Ke;
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
};


//
void print_point(ecp_t const& P) {
    std::cout << hex::b2h(P.toBytes()) << std::endl;
}




class Spake2p {
public:
    Spake2p(const bytes_t& context, const W0W1L& base) : context_(context), base_(base) {
        mbedtls_ecp_group_init(&group_);
        mbedtls_ecp_group_load(&group_, MBEDTLS_ECP_DP_SECP256R1);
    }

    ~Spake2p() {
        mbedtls_ecp_group_free(&group_);
    }

    //
    static W0W1L computeW0W1L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        //
        bytes_t pinBytes = pinToBytes(pin);

        //
        auto ws = crypto::pbkdf2(pinBytes, pbkdfParameters.salt, pbkdfParameters.iterations, PBKDF2_OUTLEN);
        if (ws.size() < PBKDF2_OUTLEN) { throw std::runtime_error("PBKDF2: недостаточная длина вывода"); }

        //
        auto slice0 = bytes_t(ws.begin(), ws.begin() + CRYPTO_W_SIZE_BYTES);
        auto slice1 = bytes_t(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.end());

        // TODO: use single group instead of recreating group
        mbedtls_ecp_group group; mbedtls_ecp_group_init(&group);
        checkMbedtlsError(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1), "ECC group load failed");

        //
        W0W1L w0w1L = {};
        w0w1L.w0 = mpi_t(slice0) % group.N;
        w0w1L.w1 = mpi_t(slice1) % group.N;
        w0w1L.L  = computeLPoint(group, w0w1L.w1);
        w0w1L.random = mpi_t().random();

        //
        mbedtls_ecp_group_free(&group);
        return w0w1L;
    }


    // aka. pA
    bytes_t computeX() { X_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getM(), base_.w0); return X_.toBytes(); }

    // aka. pB
    bytes_t computeY() { Y_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getN(), base_.w0); return Y_.toBytes(); }


    // to-server (X and L from client), aka. by Y  aka. cA
    SecretAndVerifiers computeSecretAndVerifiersFromX( bytes_t const& bX, bytes_t const& L) const {
        ecp_t Lp = ecp_t(group_).loadBytes(L);
        ecp_t X  = ecp_t(group_).loadBytes(bX);
        ecp_t Br = X  - ecp_t(group_).getM() * base_.w0;
        ecp_t Z  = Br * base_.random;
        ecp_t V  = Lp * base_.random;
        return computeSecretAndVerifiers(X, Y_, Z, V);
    }

    // to-client (Y and w1 from server), aka. by X,  aka. cB
    SecretAndVerifiers computeSecretAndVerifiersFromY( bytes_t const& bY, bigint_t const& w1b) const {
        mpi_t w1 = mpi_t(w1b);
        ecp_t Y  = ecp_t(group_).loadBytes(bY);
        ecp_t Br = Y  - ecp_t(group_).getN() * base_.w0;
        ecp_t Z  = Br * base_.random;
        ecp_t V  = Br * base_.w1;
        return computeSecretAndVerifiers(X_, Y, Z, V);
    }



private:

    //
    SecretAndVerifiers computeSecretAndVerifiers( ecp_t X, ecp_t Y, ecp_t Z, ecp_t V) const {
        //print_point(X); print_point(Y); print_point(Z); print_point(V);

        //
        bytes_t info = hex::s2b("ConfirmationKeys");

        //
        bigint_t transcript = computeTranscriptHash(X, Y, Z, V);
        bigint_t Ka = *(bigint_t*)((uint8_t*)&transcript + 0),
                 Ke = *(bigint_t*)((uint8_t*)&transcript + 16);

        //
        bigint_t KcAB = crypto::hkdf(bytes_t((uint8_t*)&Ka, ((uint8_t*)&Ka)+32), bytes_t{}, info, 32);
        bigint_t KcA = *(bigint_t*)((uint8_t*)&KcAB + 0), KcB = *(bigint_t*)((uint8_t*)&KcAB + 16); // TODO: use uint128_t

        //
        SecretAndVerifiers result = {};
        result.Ke = Ke;

        //
        result.hAY = crypto::hmac(KcA, Y.toBytes());
        result.hBX = crypto::hmac(KcB, X.toBytes());
        return result;
    }

    //
    bigint_t computeTranscriptHash(const ecp_t& X, const ecp_t& Y, const ecp_t& Z, const ecp_t& V) const {
        DataWriter writer; writer.writeBytes(context_);
        writer.writeBigint(bigint_t(0));

        // N and M write
        writer.writeBytes(ecp_t(group_).getM().toBytes(false));
        writer.writeBytes(ecp_t(group_).getN().toBytes(false));

        // X, Y, Z, V points
        writer.writeBytes(X.toBytes(false)); // pA
        writer.writeBytes( Y.toBytes(false)); // pB
        writer.writeBytes( Z.toBytes(false));
        writer.writeBytes(V.toBytes(false));

        // writing w0 and compute hash
        writer.writeBytes(hex::n2b(base_.w0));
        return crypto::hash(writer.toBytes());
    }

    //
    static bytes_t pinToBytes(uint32_t pin) {
        bytes_t pinBytes(4);
        for (size_t i = 0; i < 4; i++) {
            pinBytes[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff);
        }
        return pinBytes;
    }

    //
    static ecp_t computeLPoint(mbedtls_ecp_group const& group, mpi_t const& w1) {
        return (ecp_t(group, group.G) * w1);
    }


    //
    union { ecp_t X_, Y_; };
    mbedtls_ecp_group group_;
    bytes_t context_ = {};
    W0W1L base_ = {};
};

#endif

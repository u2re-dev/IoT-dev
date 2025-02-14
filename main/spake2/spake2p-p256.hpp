#ifndef D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE
#define D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE

//
#include "../std/types.hpp"
#include "../std/hex.hpp"

//
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

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
    bytes_t Ke;
    bytes_t hAY;
    bytes_t hBX;
};

//
struct PbkdfParameters {
    uint32_t iterations;
     bytes_t salt;
};

//
struct W0W1L {
    mpi_t w0;
    mpi_t w1;
    mpi_t random;
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

        //
        mbedtls_ecp_group group;
        mbedtls_ecp_group_init(&group);
        checkMbedtlsError(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1), "ECC group load failed");

        //
        W0W1L w0w1L = {};
        w0w1L.w0 = mpi_t(slice0) % group.N;
        w0w1L.w1 = mpi_t(slice1) % group.N;
        w0w1L.random = mpi_t().random();
        w0w1L.L = computeLPoint(group, w0w1L.w1);
        
        //
        mbedtls_ecp_group_free(&group);
        return w0w1L;
    }

    ecp_t computeX() {
        X_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getM(), base_.w0);
        return X_;//.toBytes();
    }

    ecp_t computeY() {
        Y_ = ecp_t(group_, group_.G).muladd(base_.random, ecp_t(group_).getN(), base_.w0);
        return Y_;//.toBytes();
    }






    
    // to-server (X and L from client), aka. by Y
    SecretAndVerifiers computeSecretAndVerifiersFromX( ecp_t X, /*const bytes_t& bL*/ bytes_t const& L) const {
        ecp_t Lp = ecp_t(group_).loadBytes(L);
        //ecp_t X  = ecp_t(group_).loadBytes(bX);
        ecp_t Br = X - ecp_t(group_).getM() * base_.w0;
        ecp_t Z  = Br * base_.random;
        ecp_t V  = Lp * base_.random;
        return computeSecretAndVerifiers(X, Y_, Z, V);
    }

    // to-client (Y and w1 from server), aka. by X
    SecretAndVerifiers computeSecretAndVerifiersFromY( ecp_t Y, /*const bytes_t& w1b*/ bytes_t const& w1b) const {
        mpi_t w1 = mpi_t(w1b);
        //ecp_t Y  = ecp_t(group_).loadBytes(bY);
        ecp_t Br = Y - ecp_t(group_).getN() * base_.w0;
        ecp_t Z  = Br * base_.random;
        ecp_t V  = Br * base_.w1;
        return computeSecretAndVerifiers(X_, Y, Z, V);
    }



private:

    SecretAndVerifiers computeSecretAndVerifiers( ecp_t X, ecp_t Y, ecp_t Z, ecp_t V) const {
        print_point(X);
        print_point(Y);
        print_point(Z);
        print_point(V);

        /*
        //
        bytes_t    transcript    = computeTranscriptHash(X, Y, Z, V);
        bytes_t Ka(transcript.begin()     , transcript.begin() + 16);
        bytes_t Ke(transcript.begin() + 16, transcript.begin() + 32);

        //
        bytes_t info = hex::s2b("ConfirmationKeys");
        bytes_t KcAB = crypto::hkdf(Ka, bytes_t{}, info, 32);

        //
        bytes_t KcA(KcAB.begin()     , KcAB.begin() + 16);
        bytes_t KcB(KcAB.begin() + 16, KcAB.end()       );*/

        //
        SecretAndVerifiers result;
        //result.Ke  = Ke;
        //result.hAY = crypto::hmac(KcA, Y);
        //result.hBX = crypto::hmac(KcB, X);
        return result;
    }

    //
    bytes_t computeTranscriptHash(const mbedtls_ecp_point& X, const mbedtls_ecp_point& Y, const mbedtls_ecp_point& Z, const mbedtls_ecp_point& V) const {
        DataWriter writer;
        writer.writeBytes(context_);

        // ? required
        writer.writeBytes(bytes_t{});
        writer.writeBytes(bytes_t{});

        // N and M write
        //writer.writeBytes(eccp_t::getM().toBytes(false));
        //writer.writeBytes(eccp_t::getM().toBytes(false));

        // X, Y, Z, V points
        //writer.writeBytes(X);
        //writer.writeBytes(Y);
        //writer.writeBytes(Z);
        //writer.writeBytes(V);

        // writing w0 and compute hash
        //writer.writeBytes(hex::n2b_be(w0_, 32));
        return crypto::hash(writer.toBytes());
    }




    // Преобразование PIN в байты
    static bytes_t pinToBytes(uint32_t pin) {
        bytes_t pinBytes(4);
        for (size_t i = 0; i < 4; i++) {
            pinBytes[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff);
        }
        return pinBytes;
    }

    // Вычисление точки L = w1 * G
    static ecp_t computeLPoint(mbedtls_ecp_group const& group, mpi_t const& w1) {
        return (ecp_t(group, group.G) * w1);
    }

    ecp_t X_;
    ecp_t Y_;
    mbedtls_ecp_group group_;
    bytes_t context_;
    W0W1L base_;
};

#endif /* D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE */

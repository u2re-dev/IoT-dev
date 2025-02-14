#pragma once

//
#include "../../std/types.hpp"
#include "../../bigint/hex.hpp"
#include "../../std/types.hpp"
#include "crypto.hpp"

//
#include <mbedtls/ctr_drbg.h>
#include "../../mbedtls/ecc_point_compression.h"

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
    mbedtls_mpi w0;
    mbedtls_mpi w1;
    mbedtls_mpi random;
    mbedtls_ecp_point L;
};


//
void print_point(mbedtls_ecp_point const& P) {
    bytes_t Pb(65);
    size_t len = 65;
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_ecp_point_write_binary(&grp, &P, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, Pb.data(), Pb.size());
    std::cout << hex::b2h(Pb) << std::endl;
}



//
mbedtls_ecp_point loadMPoint(mbedtls_ecp_group const& grp) {
    bytes_t Mb = hex::h2b("02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f");

    // get M point
    bytes_t Mxy(65); size_t oLen = 65;
    mbedtls_ecp_decompress(&grp, Mb.data(), Mb.size(), Mxy.data(), &oLen, Mxy.size());
    mbedtls_ecp_point M; mbedtls_ecp_point_init(&M);
    mbedtls_ecp_point_read_binary(&grp, &M, Mxy.data(), Mxy.size());

    //
    return M;
}

//
mbedtls_ecp_point loadNPoint(mbedtls_ecp_group const& grp) {
    bytes_t Nb = hex::h2b("03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49");

    // get M point
    bytes_t Nxy(65); size_t oLen = 65;
    mbedtls_ecp_decompress(&grp, Nb.data(), Nb.size(), Nxy.data(), &oLen, Nxy.size());
    mbedtls_ecp_point N; mbedtls_ecp_point_init(&N);
    mbedtls_ecp_point_read_binary(&grp, &N, Nxy.data(), Nxy.size());

    //
    return N;
}


//
class Spake2p {
public:

    // from-server generation (may be in server due of some limitations)
    static W0W1L computeW0W1L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        bytes_t pinbytes_t(4); for (size_t i = 0; i < 4; i++) { pinbytes_t[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff); }







        //
        constexpr size_t   CRYPTO_W_SIZE_BYTES = 40;
        size_t outputLen = CRYPTO_W_SIZE_BYTES * 2;

        //
        bytes_t ws = crypto::pbkdf2(pinbytes_t, pbkdfParameters.salt, pbkdfParameters.iterations, outputLen);
        if (ws.size() < outputLen) throw std::runtime_error("pbkdf2: недостаточная длина вывода");

        //
        auto slice0 = bytes_t(ws.begin()                      , ws.begin() +     CRYPTO_W_SIZE_BYTES);
        auto slice1 = bytes_t(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.begin() + 2 * CRYPTO_W_SIZE_BYTES);

        // init structure
        W0W1L w0w1L = {};
        mbedtls_mpi_init(&w0w1L.w0);
        mbedtls_mpi_init(&w0w1L.w1);
        mbedtls_mpi_init(&w0w1L.random);

        // get slices
        mbedtls_mpi_read_binary(&w0w1L.w0, slice0.data(), slice0.size());
        mbedtls_mpi_read_binary(&w0w1L.w1, slice1.data(), slice1.size());

        // get w0 and w1
        mbedtls_ecp_group grp;
        mbedtls_ecp_group_init(&grp);
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
        mbedtls_mpi_mul_mpi(&w0w1L.w0, &w0w1L.w0, &grp.P);
        mbedtls_mpi_mul_mpi(&w0w1L.w1, &w0w1L.w1, &grp.P);

        //
        mbedtls_mpi ZERO; mbedtls_mpi_init(&ZERO); mbedtls_mpi_lset(&ZERO, 0);
        mbedtls_ecp_point ECP_ZERO; mbedtls_ecp_point_init(&ECP_ZERO); mbedtls_ecp_set_zero(&ECP_ZERO);




        // ...compute random
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_mpi_fill_random(&w0w1L.random, 32, mbedtls_ctr_drbg_random, &ctr_drbg);

        // ...compute L by w1 * BASE
        mbedtls_ecp_point_init(&w0w1L.L); mbedtls_ecp_set_zero(&w0w1L.L);
        mbedtls_ecp_muladd(&grp, &w0w1L.L, &w0w1L.w1, &grp.G,   &ZERO, &ECP_ZERO);

        //
        return w0w1L;
    }

    //
    static Spake2p create(const bytes_t& context, W0W1L const& base) {
        //bigint_t random = crypto::getRandomBigint(32, eccp_t::getCurveOrder());
        return Spake2p(context, base);
    }



    // from-client
    bytes_t computeX() {
        mbedtls_ecp_group grp;
        mbedtls_ecp_group_init(&grp);
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);

        //
        auto M = loadMPoint(grp);

        //
        mbedtls_ecp_point_init(&X_); mbedtls_ecp_set_zero(&X_);
        mbedtls_ecp_muladd(&grp, &X_,   &base_.random, &grp.G,   &base_.w0, &M);

        //
        bytes_t Xb(65); size_t len = 65;
        mbedtls_ecp_point_write_binary(&grp, &X_, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, Xb.data(), Xb.size());
        return Xb;
    }

    // from-server
    bytes_t computeY() {
        mbedtls_ecp_group grp;
        mbedtls_ecp_group_init(&grp);
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);

        //
        auto N = loadNPoint(grp);

        //
        mbedtls_ecp_point_init(&Y_); mbedtls_ecp_set_zero(&Y_);
        mbedtls_ecp_muladd(&grp, &Y_,   &base_.random, &grp.G,   &base_.w0, &N);

        //
        bytes_t Yb(65); size_t len = 65;
        mbedtls_ecp_point_write_binary(&grp, &Y_, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, Yb.data(), Yb.size());
        return Yb;

        //eccp_t Br  = eccp_t::getBase() * random_;
        //eccp_t Nw0 = eccp_t::getN()    * w0_;
        //return (Br + Nw0).toBytes(false);
    }



    // to-server (X and L from client), aka. by Y
    SecretAndVerifiers computeSecretAndVerifiersFromX(const bytes_t& bX, /*const bytes_t& bL*/ mbedtls_ecp_point const& L) const {
        //eccp_t Lp = eccp_t::fromBytes(L);
        //eccp_t Xp = eccp_t::fromBytes(X); Xp.assertValidity();
        //eccp_t Br = Xp - (eccp_t::getM() * w0_); // client's           (BASE * client_random)
        //eccp_t Z  = Br * random_; // multiply to server random   (i.e. (BASE * client_random) * server_random)
        //eccp_t V  = Lp * random_; // multiply to server random   (i.e. (BASE * w1           ) * server_random)
        
        // -1 constant
        mbedtls_mpi MINUS_ONE; mbedtls_mpi_init(&MINUS_ONE); mbedtls_mpi_lset(&MINUS_ONE, -1);
        mbedtls_mpi ZERO; mbedtls_mpi_init(&ZERO); mbedtls_mpi_lset(&ZERO, 0);
        mbedtls_ecp_point ECP_ZERO; mbedtls_ecp_point_init(&ECP_ZERO); mbedtls_ecp_set_zero(&ECP_ZERO);

        //
        auto& Y = Y_;
        
        //
        mbedtls_ecp_group grp;
        mbedtls_ecp_group_init(&grp);
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);

        // load X point
        mbedtls_ecp_point X; mbedtls_ecp_point_init(&X); mbedtls_ecp_set_zero(&X);
        mbedtls_ecp_point_read_binary(&grp, &X, bX.data(), bX.size());

        // load L point
        //mbedtls_ecp_point L; mbedtls_ecp_point_init(&L);
        //mbedtls_ecp_point_read_binary(&grp, &L, bL.data(), bL.size());

        //
        auto M = loadNPoint(grp);

        //
        mbedtls_ecp_point Br; mbedtls_ecp_point_init(&Br); mbedtls_ecp_set_zero(&Br);
        mbedtls_ecp_muladd(&grp, &Br,  &MINUS_ONE, &X,   &base_.w0 , &M);   // as - (X - (M * w0))
        mbedtls_ecp_muladd(&grp, &Br,  &MINUS_ONE, &Br,  &ZERO, &ECP_ZERO); // as * (-1)

        // compute Z and V
        mbedtls_ecp_point Z; mbedtls_ecp_point_init(&Z); mbedtls_ecp_set_zero(&Z); mbedtls_ecp_muladd(&grp, &Z,  &base_.random, &Br,    &ZERO, &ECP_ZERO);
        mbedtls_ecp_point V; mbedtls_ecp_point_init(&V); mbedtls_ecp_set_zero(&V); mbedtls_ecp_muladd(&grp, &V,  &base_.random, &L ,    &ZERO, &ECP_ZERO);
        return computeSecretAndVerifiers(X, Y, Z, V);


        //
        return computeSecretAndVerifiers(X, Y, Z, V);
    }

    // to-client (Y and w1 from server), aka. by X
    SecretAndVerifiers computeSecretAndVerifiersFromY(const bytes_t& bY, /*const bytes_t& w1b*/ mbedtls_mpi const& w1) const {
        //eccp_t w1 = eccp_t::fromBytes(w1b);
        //eccp_t Yp = eccp_t::fromBytes(Y); Yp.assertValidity();
        //eccp_t Br = Yp - (eccp_t::getN() * w0_); // server's           (BASE * server_random)
        //eccp_t Z  = Br * random_; // multiply to client random   (i.e. (BASE * server_random) * client_random)
        //eccp_t V  = Br * w1;      // multiply to client w1       (i.e. (BASE * server_random) * w1           )


        // -1 constant
        mbedtls_mpi MINUS_ONE; mbedtls_mpi_init(&MINUS_ONE); mbedtls_mpi_lset(&MINUS_ONE, -1);
        mbedtls_mpi ZERO; mbedtls_mpi_init(&ZERO); mbedtls_mpi_lset(&ZERO, 0);
        mbedtls_ecp_point ECP_ZERO; mbedtls_ecp_point_init(&ECP_ZERO); mbedtls_ecp_set_zero(&ECP_ZERO);

        //
        auto& X = X_;
        
        //
        mbedtls_ecp_group grp;
        mbedtls_ecp_group_init(&grp);
        mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);

        // load Y point
        mbedtls_ecp_point Y; mbedtls_ecp_point_init(&Y); mbedtls_ecp_set_zero(&Y);
        mbedtls_ecp_point_read_binary(&grp, &Y, bY.data(), bY.size());

        // load w1
        //mbedtls_mpi_uint w1; mbedtls_mpi_init(&w1);
        //mbedtls_mpi_read_binary(&w1, w1b.data(), w1b.size());


        // use N-point
        auto N = loadNPoint(grp);

        // - (- Yp + N * w0) or  (Yp - N * w0)
        mbedtls_ecp_point Br;
        mbedtls_ecp_point_init(&Br);  mbedtls_ecp_set_zero(&Br);
        mbedtls_ecp_muladd(&grp, &Br,  &MINUS_ONE, &Y,    &base_.w0 , &N);
        mbedtls_ecp_muladd(&grp, &Br,  &MINUS_ONE, &Br,   &ZERO, &ECP_ZERO);

        // compute Z and V
        mbedtls_ecp_point Z; mbedtls_ecp_point_init(&Z); mbedtls_ecp_set_zero(&Z); mbedtls_ecp_muladd(&grp, &Z, &base_.random, &Br,   &ZERO, &ECP_ZERO);
        mbedtls_ecp_point V; mbedtls_ecp_point_init(&V); mbedtls_ecp_set_zero(&V); mbedtls_ecp_muladd(&grp, &V, &base_.w1,     &Br,   &ZERO, &ECP_ZERO);

        //
        return computeSecretAndVerifiers(X, Y, Z, V);
    }

private:
    SecretAndVerifiers computeSecretAndVerifiers(const mbedtls_ecp_point& X, const mbedtls_ecp_point& Y, const mbedtls_ecp_point& Z, const mbedtls_ecp_point& V) const {
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

    //
    Spake2p(const bytes_t& context, W0W1L const& base) : context_(context), base_(base) {};

    //
    bytes_t context_;
    W0W1L base_;

    //
    mbedtls_ecp_point X_; // in-client
    mbedtls_ecp_point Y_; // in-server

    //bigint_t random_;
    //bigint_t w0_;

    //
    //bigint_t w1_; // server-holder
    //bigint_t L_;  // client-holder
};

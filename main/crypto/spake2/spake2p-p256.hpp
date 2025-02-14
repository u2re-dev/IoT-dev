#pragma once

//
#include "../../std/types.hpp"
#include "../../bigint/hex.hpp"
#include "../../std/types.hpp"
#include "../ecc/ecc.hpp"
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
class Spake2p {
public:

    // from-server generation (may be in server due of some limitations)
    static std::pair<bytes_t, bytes_t> computeW0W1(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
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

        //
        return { w0, w1 };
    }

    // from-server generation (alternate version)
    static std::pair<bytes_t, bytes_t> computeW0L(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        const auto [w0, w1] = computeW0W1(pbkdfParameters, pin);
        return { w0, computeL(w1) };
    }

    // from-client generation (just only L generate)
    static bytes_t computeL(bytes_t const& w1) {
        //return (eccp_t::getBase() * w1).toBytes(false);
    }

    //
    static Spake2p create(const bytes_t& context, bigint_t w0) {
        //bigint_t random = crypto::getRandomBigint(32, eccp_t::getCurveOrder());
        return Spake2p(context, random, w0);
    }



    // from-client
    bytes_t computeX() const {
        //eccp_t Br  = eccp_t::getBase() * random_;
        //eccp_t Mw0 = eccp_t::getM()    * w0_;
        //return (Br + Mw0).toBytes(false);
    }

    // from-server
    bytes_t computeY() const {
        //eccp_t Br  = eccp_t::getBase() * random_;
        //eccp_t Nw0 = eccp_t::getN()    * w0_;
        //return (Br + Nw0).toBytes(false);
    }



    // to-server (X and L from client), aka. by Y
    SecretAndVerifiers computeSecretAndVerifiersFromX(const bytes_t& Y,   const bytes_t& X, const bytes_t& L) const {
        //eccp_t Lp = eccp_t::fromBytes(L);
        //eccp_t Xp = eccp_t::fromBytes(X); Xp.assertValidity();
        //eccp_t Br = Xp - (eccp_t::getM() * w0_); // client's           (BASE * client_random)
        //eccp_t Z  = Br * random_; // multiply to server random   (i.e. (BASE * client_random) * server_random)
        //eccp_t V  = Lp * random_; // multiply to server random   (i.e. (BASE * w1           ) * server_random)

        
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

    // to-client (Y and w1 from server), aka. by X
    SecretAndVerifiers computeSecretAndVerifiersFromY(const bytes_t& X,   const bytes_t& Y, const bigint_t& w1) const {
        //eccp_t w1 = eccp_t::fromBytes(w1b);
        //eccp_t Yp = eccp_t::fromBytes(Y); Yp.assertValidity();
        //eccp_t Br = Yp - (eccp_t::getN() * w0_); // server's           (BASE * server_random)
        //eccp_t Z  = Br * random_; // multiply to client random   (i.e. (BASE * server_random) * client_random)
        //eccp_t V  = Br * w1;      // multiply to client w1       (i.e. (BASE * server_random) * w1           )


        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

private:
    SecretAndVerifiers computeSecretAndVerifiers(const bytes_t& X, const bytes_t& Y, const bytes_t& Z, const bytes_t& V) const {
        std::cout << hex::b2h(X) << std::endl;
        std::cout << hex::b2h(Y) << std::endl;
        std::cout << hex::b2h(Z) << std::endl;
        std::cout << hex::b2h(V) << std::endl;

        //
        bytes_t    transcript    = computeTranscriptHash(X, Y, Z, V);
        bytes_t Ka(transcript.begin()     , transcript.begin() + 16);
        bytes_t Ke(transcript.begin() + 16, transcript.begin() + 32);

        //
        bytes_t info = hex::s2b("ConfirmationKeys");
        bytes_t KcAB = crypto::hkdf(Ka, bytes_t{}, info, 32);

        //
        bytes_t KcA(KcAB.begin()     , KcAB.begin() + 16);
        bytes_t KcB(KcAB.begin() + 16, KcAB.end()       );

        //
        SecretAndVerifiers result;
        result.Ke  = Ke;
        result.hAY = crypto::hmac(KcA, Y);
        result.hBX = crypto::hmac(KcB, X);
        return result;
    }

    //
    bytes_t computeTranscriptHash(const bytes_t& X, const bytes_t& Y, const bytes_t& Z, const bytes_t& V) const {
        DataWriter writer;
        writer.writeBytes(context_);

        // ? required
        writer.writeBytes(bytes_t{});
        writer.writeBytes(bytes_t{});

        // N and M write
        writer.writeBytes(eccp_t::getM().toBytes(false));
        writer.writeBytes(eccp_t::getM().toBytes(false));

        // X, Y, Z, V points
        writer.writeBytes(X);
        writer.writeBytes(Y);
        writer.writeBytes(Z);
        writer.writeBytes(V);

        // writing w0 and compute hash
        writer.writeBytes(hex::n2b_be(w0_, 32));
        return crypto::hash(writer.toBytes());
    }

    //
    Spake2p(const bytes_t& context, bigint_t random, bigint_t w0) : context_(context), random_(random), w0_(w0) {}

    //
      bytes_t context_;
    bigint_t random_;
    bigint_t w0_;

    //
    //bigint_t w1_; // server-holder
    //bigint_t L_;  // client-holder
};

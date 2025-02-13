#pragma once

//
#include "../../std/types.hpp"
#include "../../std/hex.hpp"
#include "../ecc/ecc.hpp"

//
constexpr uint8_t  H_VERSION   = 0x01;
constexpr uint16_t SER_VERSION = 0x0001;

//
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN = CRYPTO_W_SIZE_BYTES * 2;

//
struct SecretAndVerifiers {
    bytes_t Ke;  // Ключ Ke (обычно 16 байт)
    bytes_t hAY; // HMAC от Y с ключом KcA (например, 16 байт)
    bytes_t hBX; // HMAC от X с ключом KcB (например, 16 байт)
};

//
struct PbkdfParameters {
    uint32_t iterations;
     bytes_t salt;
};

//
class Spake2p {
public: //
    static std::pair<uint256_t, uint256_t> computeW0W1(const PbkdfParameters& pbkdfParameters, uint32_t pin) {
        bytes_t pinbytes_t(4);
        for (size_t i = 0; i < 4; i++) { pinbytes_t[i] = static_cast<uint8_t>((pin >> (8 * i)) & 0xff); }

        //
        constexpr size_t   CRYPTO_W_SIZE_BYTES = 40;
        size_t outputLen = CRYPTO_W_SIZE_BYTES * 2;

        //
        bytes_t ws = Crypto::pbkdf2(pinbytes_t, pbkdfParameters.salt, pbkdfParameters.iterations, outputLen);
        if (ws.size() < outputLen) throw std::runtime_error("pbkdf2: недостаточная длина вывода");

        //
        uint256_t w0 = hex::bytesToBigint(bytes_t(ws.begin()                      , ws.begin() +     CRYPTO_W_SIZE_BYTES));
        uint256_t w1 = hex::bytesToBigint(bytes_t(ws.begin() + CRYPTO_W_SIZE_BYTES, ws.begin() + 2 * CRYPTO_W_SIZE_BYTES));

        //
        w0 = mod(w0, eccp_t::getCurveOrder());
        w1 = mod(w1, eccp_t::getCurveOrder());
        return { w0, w1 };
    }

    //
    static std::pair<uint256_t, bytes_t> computeW0L( std::pair<uint256_t, uint256_t> const& w0w1) {
        auto [w0, w1] = w0w1;
        bytes_t L = (eccp_t::getBase() * w1).toBytes(false);
        return { w0, L };
    }

    //
    static Spake2p create(const bytes_t& context, uint256_t w0) {
        uint256_t random = Crypto::getRandombigint_t(32, getCurveOrder());
        return Spake2p(context, random, w0);
    }

    //
    bytes_t computeX() const {
        eccp_t part1 = eccp_t::getBase() * random_;
        eccp_t part2 = eccp_t::getM()    * w0_;
        return (part1 + part2).toBytes(false);
    }

    //
    bytes_t computeY() const {
        eccp_t part1 = eccp_t::getBase() * random_;
        eccp_t part2 = eccp_t::getN()    * w0_;
        return (part1 + part2).toBytes(false);
    }

    //
    SecretAndVerifiers computeSecretAndVerifiersFromY(uint256_t w1, const bytes_t& X, const bytes_t& Y) const {
        eccp_t Yp   = eccp_t::fromBytes(Y); Yp.assertValidity();
        eccp_t yNwo = Yp - (eccp_t::getN() * w0_);
        eccp_t Z    = yNwo * random_;
        eccp_t V    = yNwo * w1;
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

    //
    SecretAndVerifiers computeSecretAndVerifiersFromX(const bytes_t& L, const bytes_t& X, const bytes_t& Y) const {
        eccp_t Xp = eccp_t::fromBytes(X); Xp.assertValidity();
        eccp_t Lp = eccp_t::fromBytes(L);
        eccp_t Z  = (Xp - (eccp_t::getM() * w0_)) * random_;
        eccp_t V  =  Lp * random_;
        return computeSecretAndVerifiers(X, Y, Z.toBytes(false), V.toBytes(false));
    }

private:
    SecretAndVerifiers computeSecretAndVerifiers(const bytes_t& X, const bytes_t& Y, const bytes_t& Z, const bytes_t& V) const {
        bytes_t transcript = computeTranscriptHash(X, Y, Z, V);
        bytes_t Ka(transcript.begin()     , transcript.begin() + 16);
        bytes_t Ke(transcript.begin() + 16, transcript.begin() + 32);

        //
        bytes_t info = stringToBytes("ConfirmationKeys");
        bytes_t KcAB = Crypto::hkdf(Ka, bytes_t{}, info, 32);

        //
        bytes_t KcA(KcAB.begin()     , KcAB.begin() + 16);
        bytes_t KcB(KcAB.begin() + 16, KcAB.end()       );

        //
        SecretAndVerifiers result;
        result.Ke  = Ke;
        result.hAY = Crypto::hmac(KcA, Y);
        result.hBX = Crypto::hmac(KcB, X);
        return result;
    }

    //
    bytes_t computeTranscriptHash(const bytes_t& X, const bytes_t& Y, const bytes_t& Z, const bytes_t& V) const {
        DataWriter writer;
        addToContext(writer, context_);

        // ? required
        addToContext(writer, bytes_t{});
        addToContext(writer, bytes_t{});

        // N and M write
        addToContext(writer, M().toBytes(false));
        addToContext(writer, N().toBytes(false));

        // X, Y, Z, V points
        addToContext(writer, X);
        addToContext(writer, Y);
        addToContext(writer, Z);
        addToContext(writer, V);

        // writing w0 and compute hash
        addToContext(writer, numberToBytesBE(w0_, 32));
        return Crypto::hash(writer.toBytes());
    }

    //
    static void addToContext(DataWriter& writer, const bytes_t& data) {
        writer.writeBytes(data);
    }

    //
    Spake2p(const bytes_t& context, uint256_t random, uint256_t w0) : context_(context), random_(random), w0_(w0) {}

    //
    bytes_t context_;
    uint256_t random_;
    uint256_t w0_;
};

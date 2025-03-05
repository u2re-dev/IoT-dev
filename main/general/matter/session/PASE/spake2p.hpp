#ifndef D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE
#define D4F62BFA_CBF0_4D2C_A09D_95C7B1FF78AE

//
#include <iostream>

//
#include <std/types.hpp>
#include <std/hex.hpp>

//
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

//
#include <raii/misc.hpp>
#include <raii/mpi.hpp>
#include <raii/ecp.hpp>
#include <raii/group.hpp>
#include <raii/crypto.hpp>

//
constexpr  uint8_t H_VERSION             = 0x01;
constexpr uint16_t SER_VERSION           = 0x0001;
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES     = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN           = CRYPTO_W_SIZE_BYTES     * 2;

//
struct HKDF_HMAC { intx::uint128 Ke = 0; bigint_t hAY = 0, hBX = 0; };
struct PBKDFParameters { uint16_t iterations = 0; bigint_t salt = 0; };
struct W0W1L { ecp_t L; mpi_t w0, w1, rand; };

//
using uncomp_t = bytespan_t;
class Spake2p {
public:
    inline ~Spake2p() {}
    inline Spake2p(Spake2p const& another) : X_(another.X_), group_(another.group_), context_(another.context_), base_(another.base_) {}
    inline Spake2p(PBKDFParameters const& pbkdfParameters, uint32_t const& pin, bigint_t const& context) { init(pbkdfParameters, pin, context); }

    //
    inline ecp_t parseECP (uint8_t const* stream, size_t const& length) { return group_.make(bytespan_t(stream, length)); }
    inline ecp_t parseECP (bytespan_t const& bytes) { return group_.make(bytes); }
    inline void init(const PBKDFParameters& pbkdfParameters, uint32_t const& pin, bigint_t const& context) {
        base_    = computeW0W1L(group_, pbkdfParameters, pin);
        context_ = context;
    }

    //
    static bigint_t computeContextHash(bytespan_t const& req, bytespan_t const& res) {
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts_ret(&ctx, 0);

        //
        const auto name = hex::s2b("CHIP PAKE V1 Commissioning");
        mbedtls_sha256_update_ret(&ctx, name->data(), name->size());
        mbedtls_sha256_update_ret(&ctx, req->data(), req->size());
        mbedtls_sha256_update_ret(&ctx, res->data(), res->size());

        //
        auto out = bigint_t(0);
        checkMbedtlsError(mbedtls_sha256_finish_ret(&ctx, reinterpret_cast<uint8_t*>(&out)), "Failed to compute Hash");
        return out;
    }


    // Client
    inline ecp_t& computeY() { Y_ = group_.getGp().muladd(base_.rand, group_.getNp(), base_.w0); return Y_; }
    inline HKDF_HMAC computeHKDFFromX( uncomp_t const& bX ) {
        ecp_t X  = group_.make(bX);
        ecp_t Lp = base_.L;
        ecp_t Br = X  - group_.getMp() * base_.w0; // with foreign factor
        ecp_t Z  = Br * base_.rand; // random factor crossed
        ecp_t V  = Lp * base_.rand; // always Device-side factor (own)
        return computeHKDF(X, Y_, Z, V);
    }


    // Initiator //!unused
    inline ecp_t& computeX() { X_ = group_.getGp().muladd(base_.rand, group_.getMp(), base_.w0); return X_; }
    inline HKDF_HMAC computeHKDFFromY( uncomp_t const& bY ) {
        ecp_t Y  = group_.make(bY);
        ecp_t Br = Y  - group_.getNp() * base_.w0; // with foreign factor
        ecp_t Z  = Br * base_.rand; // random factor crossed
        ecp_t V  = Br * base_.w1;     // always Device-side factor (foreign)
        return computeHKDF(X_, Y, Z, V);
    }


private:

    //
    inline HKDF_HMAC computeHKDF( ecp_t X, ecp_t Y, ecp_t Z, ecp_t V) const {
        auto transcript = crypto::hash(computeTranscript(X, Y, Z, V));

        // TODO: use better references (don't copy memory)
        decltype(auto) Ka = intx::uint128(transcript >> uint(0)), Ke = intx::uint128(transcript >> uint(128));

        //
        auto info = hex::s2b("ConfirmationKeys");
        auto KcAB = crypto::hkdf(Ka, info);

        // TODO: use better references (don't copy memory)
        decltype(auto) KcA = intx::uint128(KcAB >> uint(0)), KcB = intx::uint128(KcAB >> uint(128));

        //
        HKDF_HMAC result = {};
        result.Ke  = Ke;
        result.hAY = crypto::hmac(KcA, Y);
        result.hBX = crypto::hmac(KcB, X);

        //
        std::cout << hex::b2h(hex::n2b(result.hAY)) << std::endl;
        return result;
    }

    //
    inline bytespan_t computeTranscript(const ecp_t& X, const ecp_t& Y, const ecp_t& Z, const ecp_t& V) const {
        writer_t writer;
        writer.writeUInt64(32); writer.writeBigInt(context_);
        writer.writeUInt64(0); writer.writeUInt64(0); // identifiers

        // N and M write
        writer.writeUInt64(65); writer.writeBytes(group_.getMp());
        writer.writeUInt64(65); writer.writeBytes(group_.getNp());

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
    static ecp_t computeLPoint(ecp_group_t& group, mpi_t const& w1) {
        return std::move(group.getGp() * w1);
    }

    //
    static W0W1L computeW0W1L(ecp_group_t& group, const PBKDFParameters& pbkdfParameters, uint32_t const& pin) {
        decltype(auto) ws = crypto::pbkdf2(reinterpret_cast<uint8_t const*>(&pin), 4, pbkdfParameters.salt, pbkdfParameters.iterations, PBKDF2_OUTLEN);
        if (ws->size() < PBKDF2_OUTLEN) { throw std::runtime_error("PBKDF2: not enough length"); }

        //
        W0W1L w0w1L = {};
        w0w1L.w0    = mpi_t(bytespan_t(ws->data(), CRYPTO_W_SIZE_BYTES)) % group.getN(),
        w0w1L.w1    = mpi_t(bytespan_t(ws->data() + CRYPTO_W_SIZE_BYTES, CRYPTO_W_SIZE_BYTES)) % group.getN();
        w0w1L.L     = computeLPoint(group, w0w1L.w1);
        w0w1L.rand  = mpi_t().random() % group.getP();
        return w0w1L;
    }

    //
    union { ecp_t X_, Y_; };
    ecp_group_t group_ = {};
    bigint_t context_ = {};
    W0W1L base_ = {};
};

//
#endif

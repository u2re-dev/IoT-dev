#ifndef C657BD94_4904_4B38_AFF9_E6267F358CAC
#define C657BD94_4904_4B38_AFF9_E6267F358CAC

//
#include <mbedtls/ctr_drbg.h>

//
#include <std/types.hpp>
#include <std/hex.hpp>

//
#include "../mbedtls/ecc_point_compression.h"

//
#include "./mpi.hpp"
#include "./misc.hpp"
#include "./ecp.hpp"

//
class ecp_group_t {
public:
    inline ecp_group_t(mbedtls_ecp_group const& group) : group_(group) {}
    inline ecp_group_t() {
        mbedtls_ecp_group_init(&group_);
        mbedtls_ecp_group_load(&group_, MBEDTLS_ECP_DP_SECP256R1);
        {   //
            const auto bt = hex::h2b("02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f");
            size_t oLen = 65; auto bytes_ = make_bytes(oLen);
            checkMbedtlsError( mbedtls_ecp_decompress(&group_, bt->data(), bt->size(), bytes_->data(), &oLen, bytes_->size()), "Failed to decompress M point" );
            (M_point = ecp_t(group_, bytes_));
        } { //
            const auto bt = hex::h2b("03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49");
            size_t oLen = 65; auto bytes_ = make_bytes(oLen);
            checkMbedtlsError( mbedtls_ecp_decompress(&group_, bt->data(), bt->size(), bytes_->data(), &oLen, bytes_->size()), "Failed to decompress N point" );
            (N_point = ecp_t(group_, bytes_));
        }
        { G_point = ecp_t(group_, group_.G); }
    }

    //
    inline ~ecp_group_t() { mbedtls_ecp_group_free(&group_); }

    //
    inline operator mbedtls_ecp_group& () { return group_; };
    inline operator mbedtls_ecp_group const& () const { return group_; };

    //
    template<typename... Args>
    inline ecp_t make(Args... args) {
        return ecp_t(group_, std::forward<Args>(args)...);
    }

    //
    inline ecp_t const& getMp() const { return M_point; }
    inline ecp_t const& getNp() const { return N_point; }
    inline ecp_t const& getGp() const { return G_point; }

    //
    inline ecp_t& getMp() { return M_point; }
    inline ecp_t& getNp() { return N_point; }
    inline ecp_t& getGp() { return G_point; }

    //
    inline mpi_t getP() const { return mpi_t(group_.P); }
    inline mpi_t getN() const { return mpi_t(group_.N); }


private: //
    mbedtls_ecp_group group_ = {};
    ecp_t N_point = {};
    ecp_t M_point = {};
    ecp_t G_point = {};
};


#endif /* C657BD94_4904_4B38_AFF9_E6267F358CAC */

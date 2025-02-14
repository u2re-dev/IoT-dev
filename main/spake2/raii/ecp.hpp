#ifndef F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0
#define F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0

//
#include <mbedtls/ctr_drbg.h>

//
#include "../crypto.hpp"
#include "../mbedtls/ecc_point_compression.h"
#include "../../std/hex.hpp"

//
#include "./mpi.hpp"
#include "./misc.hpp"

//
class ecp_t {
public:
    ecp_t(mbedtls_ecp_group const& group, bytes_t const& bytes)           : group_(group) { mbedtls_ecp_point_init(&point_); loadBytes(bytes); }
    ecp_t(mbedtls_ecp_group const& group, mbedtls_ecp_point const& point) : group_(group) { mbedtls_ecp_point_init(&point_); mbedtls_ecp_copy(&point_, &point);  }
    ecp_t(mbedtls_ecp_group const& group)                                 : group_(group) { mbedtls_ecp_point_init(&point_); mbedtls_ecp_set_zero(&point_); }
    //ecp_t(mbedtls_ecp_point const& point)                                 : point_(point) { mbedtls_ecp_point_init(&point_); mbedtls_ecp_copy(&point_, &point); }
    ecp_t() { mbedtls_ecp_point_init(&point_); mbedtls_ecp_set_zero(&point_); }
    ecp_t(ecp_t const& a) : group_(a.group_) { mbedtls_ecp_point_init(&point_); mbedtls_ecp_copy(&point_, &a.point_); }

    //
    ~ecp_t() { mbedtls_ecp_point_free(&point_); }

    //
    mbedtls_ecp_point* get() { return &point_; }
    const mbedtls_ecp_point* get() const { return &point_; }

    //
    operator mbedtls_ecp_point*() { return &point_; }
    operator mbedtls_ecp_point const*() const { return &point_; }

    //
    ecp_t operator -() { return neg(); }

    //
    ecp_t operator *(mpi_t const& a) { return mul(a); }
    ecp_t operator +(ecp_t const& a) { return add(a); }
    ecp_t operator -(ecp_t const& a) { return sub(a); }
    ecp_t& operator =(ecp_t const& a) {
        group_ = a.group_, 
        mbedtls_ecp_point_init(&point_);
        mbedtls_ecp_copy(&point_, &a.point_);
        return *this; 
    }


    //
    ecp_t neg() {
        ecp_t c(group_); mpi_t z(1); mpi_t m(-1); ecp_t zp(group_); // initial-values
        vid(mbedtls_ecp_mul(&group_, c,  m,&point_, nullptr, nullptr));
        return c;
    }

    //
    ecp_t mul(mpi_t const& a) {
        ecp_t c(group_); ecp_t zp(group_); mpi_t z(1); // zero-values
        vid(mbedtls_ecp_mul(&group_, c,  a,&point_, nullptr, nullptr/*,   z,zp*/));
        //vid(mbedtls_ecp_muladd(&group_, c,  a,&point_,   z,zp));
        return c;
    }

    //
    ecp_t add(ecp_t const& a) {
        ecp_t c(group_); mpi_t o(1); // initial-values
        vid(mbedtls_ecp_muladd(&group_, c,  o,&point_,   o,a));
        return c;
    }

    //
    ecp_t sub(ecp_t const& a) {
        ecp_t c(group_); mpi_t o(1); mpi_t m(-1); // initial-values
        vid(mbedtls_ecp_muladd(&group_, c,  o,&point_,   m,a));
        return c;
    }

    //
    ecp_t muladd(mpi_t const& am,   ecp_t const& b, mpi_t const& bm = mpi_t(1)) {
        ecp_t c(group_); mpi_t o(1); // initial-values
        vid(mbedtls_ecp_muladd(&group_, c,  am,&point_,   bm,b));
        return c;
    }


    //
    ecp_t& zero() {
        checkMbedtlsError(mbedtls_ecp_set_zero(&point_), "invalid point");
        return *this;
    }

    //
    ecp_t getG() {
        return ecp_t(group_, group_.G);
    }

    
    //
    bytes_t toBytes() const {
        bytes_t buffer(65); size_t len = 65;
        checkMbedtlsError(mbedtls_ecp_point_write_binary(&group_, &point_, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buffer.data(), buffer.size()), "Failed to write point to bytes");
        return buffer;
    }

    
    //
    ecp_t& loadBytes(bytes_t const& data ) { 
        return vid(mbedtls_ecp_point_read_binary(&group_, &point_, data.data(), data.size()), "bytes loading failed"); 
    }
    ecp_t& loadHex  (std::string const& h) { return loadBytes(hex::h2b(h)); }



    //
    ecp_t& getM() {
        bytes_t bt = hex::h2b("02886e2f97ace46e55ba9dd7242579f2993b64e16ef3dcab95afd497333d8fa12f");
        bytes_t xy(65); size_t oLen = 65;

        //
        checkMbedtlsError( mbedtls_ecp_decompress(&group_, bt.data(), bt.size(), xy.data(), &oLen, xy.size()), "Failed to decompress M point" );
        checkMbedtlsError( mbedtls_ecp_point_read_binary(&group_, &point_, xy.data(), xy.size()), "Failed to read M point" );

        //
        return *this;
    }

    //
    ecp_t& getN() {
        bytes_t bt = hex::h2b("03d8bbd6c639c62937b04d997f38c3770719c629d7014d49a24b4f98baa1292b49");
        bytes_t xy(65); size_t oLen = 65;

        //
        checkMbedtlsError( mbedtls_ecp_decompress(&group_, bt.data(), bt.size(), xy.data(), &oLen, xy.size()), "Failed to decompress N point" );
        checkMbedtlsError( mbedtls_ecp_point_read_binary(&group_, &point_, xy.data(), xy.size()), "Failed to read N point" );

        //
        return *this;
    }


    ecp_t& vid (auto const& status, std::string const& message = "invalid operation") {
        checkMbedtlsError(status, message);
        return *this;
    }

    ecp_t const& vid (auto const& status, std::string const& message = "invalid operation") const {
        checkMbedtlsError(status, message);
        return *this;
    }

protected:
    mbedtls_ecp_group group_;
    mbedtls_ecp_point point_;
};


#endif /* F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0 */

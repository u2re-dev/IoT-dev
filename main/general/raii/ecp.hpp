#ifndef F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0
#define F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0

//
#include <mbedtls/ctr_drbg.h>

//
#include <std/types.hpp>
#include <std/hex.hpp>

//
#include "./mpi.hpp"
#include "./misc.hpp"

//
class ecp_group_t;
class ecp_t {
public:
    friend ecp_group_t;

    //
protected:
    explicit ecp_t(mbedtls_ecp_group& group, bytes_t const& bytes)           : group_(&group) { mbedtls_ecp_point_init(&point_); loadBytes(bytes); }
    ecp_t(mbedtls_ecp_group& group, bytespan_t const& bytes)        : group_(&group) { mbedtls_ecp_point_init(&point_); loadBytes(bytes); }
    ecp_t(mbedtls_ecp_group& group, mbedtls_ecp_point const& point) : group_(&group) { mbedtls_ecp_point_init(&point_); mbedtls_ecp_copy(&point_, &point);  }
    ecp_t(mbedtls_ecp_group& group)                                 : group_(&group) { mbedtls_ecp_point_init(&point_); mbedtls_ecp_set_zero(&point_); }

public:
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

    // convert to uncompressed value
    operator bytespan_t() const { return toBytes(false); }
    operator bytespan_t() { return bytes_ ? bytes_ : toBytes(false); }
    explicit operator bytes_t&() { return toBytes(false); }
    explicit operator bool() const { return !!group_; };

    //
    ecp_t operator -() { return neg(); }
    ecp_t operator *(mpi_t const& a) { return mul(a); }
    ecp_t operator +(ecp_t const& a) { return add(a); }
    ecp_t operator -(ecp_t const& a) { return sub(a); }

    //
    ecp_t neg() {
        ecp_t c(*group_); mpi_t z(1); mpi_t m(-1); ecp_t zp(*group_); // initial-values
        vid(mbedtls_ecp_mul(group_, c,  m,&point_, nullptr, nullptr));
        return c;
    }

    //
    ecp_t mul(mpi_t const& a) {
        ecp_t c(*group_); ecp_t zp(*group_); mpi_t z(1); // zero-values
        vid(mbedtls_ecp_mul(group_, c,  a,&point_, nullptr, nullptr/*,   z,zp*/));
        //vid(mbedtls_ecp_muladd(group_, c,  a,&point_,   z,zp));
        return c;
    }

    //
    ecp_t add(ecp_t const& a) {
        ecp_t c(*group_); mpi_t o(1); // initial-values
        vid(mbedtls_ecp_muladd(group_, c,  o,&point_,   o,a));
        return c;
    }

    //
    ecp_t sub(ecp_t const& a) {
        ecp_t c(*group_); mpi_t o(1); mpi_t m(-1); // initial-values
        vid(mbedtls_ecp_muladd(group_, c,  o,&point_,   m,a));
        return c;
    }

    //
    ecp_t muladd(mpi_t const& am,   ecp_t const& b, mpi_t const& bm = mpi_t(1)) {
        ecp_t c(*group_); mpi_t o(1); // initial-values
        vid(mbedtls_ecp_muladd(group_, c,  am,&point_,   bm,b));
        return c;
    }


    //
    ecp_t& operator =(bytespan_t const& a) {
        mbedtls_ecp_point_init(&point_);
        loadBytes(a);
        return *this;
    }

    //
    ecp_t& operator =(bytes_t const& a) {
        mbedtls_ecp_point_init(&point_);
        loadBytes(a);
        return *this;
    }

    //
    ecp_t& operator =(ecp_t const& a) {
        group_ = a.group_,
        mbedtls_ecp_point_init(&point_);
        mbedtls_ecp_copy(&point_, &a.point_);
        return *this;
    }



    //
    ecp_t& zero() { checkMbedtlsError(mbedtls_ecp_set_zero(&point_), "invalid point"); return *this; }
    ecp_t getG() { return ecp_t(*group_, group_->G); }

    // for const version
    bytes_t toBytes(bool compressed = false) const {
        size_t len = compressed ? 33 : 65; auto bytes_ = make_bytes(len);
        checkMbedtlsError(mbedtls_ecp_point_write_binary(group_, &point_, compressed ? MBEDTLS_ECP_PF_COMPRESSED : MBEDTLS_ECP_PF_UNCOMPRESSED, &len, bytes_->data(), bytes_->size()), "Failed to write point to bytes");
        return bytes_;
    }

    //
    bytes_t& toBytes(bool compressed = false) {
        size_t len = compressed ? 33 : 65; if (!bytes_) { bytes_ = make_bytes(len); };
        checkMbedtlsError(mbedtls_ecp_point_write_binary(group_, &point_, compressed ? MBEDTLS_ECP_PF_COMPRESSED : MBEDTLS_ECP_PF_UNCOMPRESSED, &len, bytes_->data(), bytes_->size()), "Failed to write point to bytes");
        return bytes_;
    }

    //
    ecp_t& loadBytes(bytes_t const& data ) { bytes_ = data; return vid(mbedtls_ecp_point_read_binary(group_, &point_, data->data(), data->size()), "bytes loading failed"); }
    ecp_t& loadBytes(uint8_t const* data, size_t const& length) { return vid(mbedtls_ecp_point_read_binary(group_, &point_, data, length), "bytes loading failed"); }
    ecp_t& loadBytes(bytespan_t const& data ) { bytes_ = data.holder(); return vid(mbedtls_ecp_point_read_binary(group_, &point_, data->data(), data->size()), "bytes loading failed"); }
    ecp_t& loadHex  (std::string const& h) { return loadBytes(hex::h2b(h)); }

    //
    ecp_t& vid (auto const& status, std::string const& message = "invalid operation") { checkMbedtlsError(status, message); return *this; }
    ecp_t const& vid (auto const& status, std::string const& message = "invalid operation") const { checkMbedtlsError(status, message); return *this; }

    //
protected:
    mbedtls_ecp_group* group_ = nullptr;
    mbedtls_ecp_point point_ = {};
    bytes_t bytes_ = {};
};

#endif

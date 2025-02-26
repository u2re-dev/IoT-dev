#ifndef C1AC2A76_36C0_483F_8C2E_21C70BA5DB80
#define C1AC2A76_36C0_483F_8C2E_21C70BA5DB80

//
#include <mbedtls/bignum.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

//
#include <std/types.hpp>
#include <std/hex.hpp>

//
#include "./misc.hpp"

//
class mpi_t {
public:
    mpi_t(int  a = 0) { mbedtls_mpi_init(&mpi_); vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t(long a) { mbedtls_mpi_init(&mpi_); vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t(uint a) { mbedtls_mpi_init(&mpi_); vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t(bytespan_t const& a) { mbedtls_mpi_init(&mpi_); loadBytes(a); }
    mpi_t(uint8_t const* a, size_t const& len) { mbedtls_mpi_init(&mpi_); loadBytes(a, len); }

    //
    mpi_t(mbedtls_mpi const& mpi) : mpi_() { mbedtls_mpi_init(&mpi_); mbedtls_mpi_copy(&mpi_, &mpi); }
    mpi_t(mbedtls_mpi const* mpi) : mpi_() { mbedtls_mpi_init(&mpi_); mbedtls_mpi_copy(&mpi_,  mpi); }
    mpi_t(mpi_t const& mpi) { mbedtls_mpi_init(&mpi_); mbedtls_mpi_copy(&mpi_,  mpi); }

    //
    ~mpi_t() { mbedtls_mpi_free(&mpi_); }

    //
    mpi_t& loadBytes(uint8_t const* a, size_t const& len) { return vid(mbedtls_mpi_read_binary(&mpi_, a, len), "bytes loading failed"); }
    mpi_t& loadBytes(bytespan_t const& data ) { return loadBytes(data->data(), data->size()); }
    mpi_t& loadHex  (std::string const& h) { return loadBytes(hex::h2b(h)); }

    // bigint construction
    bytespan_t toBytes(size_t const& len = 32) const { bytespan_t bytes = make_bytes(len); mbedtls_mpi_write_binary(&mpi_, bytes->data(), bytes->size()); return bytes; }
    operator bigint_t() const { bigint_t x = 0; mbedtls_mpi_write_binary(&mpi_, reinterpret_cast<uint8_t*>(&x), sizeof(x)); return x; }

    // construction from bigint
    mpi_t(bigint_t const& a) { mbedtls_mpi_init(&mpi_); loadBytes(reinterpret_cast<uint8_t const*>(&a), sizeof(a)); }
    mpi_t& operator=(bigint_t const& a) { return loadBytes((uint8_t const*)&a, sizeof(a)); }

    // assign-ops
    mpi_t& operator=( int  const& a) { return vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t& operator=(uint  const& a) { return vid(mbedtls_mpi_lset(&mpi_, long(a))); }
    mpi_t& operator=(mpi_t const& a) { return vid(mbedtls_mpi_copy(&mpi_, a)); }


    // uint-based
    mpi_t& operator+=(uint const& a) { return vid(mbedtls_mpi_add_int(&mpi_, &mpi_, a)); }
    mpi_t& operator-=(uint const& a) { return vid(mbedtls_mpi_sub_int(&mpi_, &mpi_, a)); }
    mpi_t& operator*=(uint const& a) { return vid(mbedtls_mpi_mul_int(&mpi_, &mpi_, a)); }
    mpi_t& operator/=(uint const& a) { return vid(mbedtls_mpi_div_int(&mpi_, nullptr, &mpi_, a)); }
    //mpi_t& operator%=(uint const& a) { return vid(mbedtls_mpi_mod_int(&mpi_, &mpi_, a)); }

    // long-based
    mpi_t& operator+=(long const& a) { return vid(mbedtls_mpi_add_int(&mpi_, &mpi_, a)); }
    mpi_t& operator-=(long const& a) { return vid(mbedtls_mpi_sub_int(&mpi_, &mpi_, a)); }
    mpi_t& operator*=(long const& a) { return vid(mbedtls_mpi_mul_int(&mpi_, &mpi_, a)); }
    mpi_t& operator/=(long const& a) { return vid(mbedtls_mpi_div_int(&mpi_, nullptr, &mpi_, a)); }
    //mpi_t& operator%=(long const& a) { return vid(mbedtls_mpi_mod_int(&mpi_, &mpi_, a)); }

    // int-based
    mpi_t& operator+=(int const& a) { return vid(mbedtls_mpi_add_int(&mpi_, &mpi_, a)); }
    mpi_t& operator-=(int const& a) { return vid(mbedtls_mpi_sub_int(&mpi_, &mpi_, a)); }
    mpi_t& operator*=(int const& a) { return vid(mbedtls_mpi_mul_int(&mpi_, &mpi_, a)); }
    mpi_t& operator/=(int const& a) { return vid(mbedtls_mpi_div_int(&mpi_, nullptr, &mpi_, a)); }
    //mpi_t& operator%=(int const& a) { return vid(mbedtls_mpi_mod_int(&mpi_, &mpi_, a)); }

    // MPI-based
    mpi_t& operator+=(mpi_t const& a) { return vid(mbedtls_mpi_add_mpi(&mpi_, &mpi_, a)); }
    mpi_t& operator-=(mpi_t const& a) { return vid(mbedtls_mpi_sub_mpi(&mpi_, &mpi_, a)); }
    mpi_t& operator*=(mpi_t const& a) { return vid(mbedtls_mpi_mul_mpi(&mpi_, &mpi_, a)); }
    mpi_t& operator/=(mpi_t const& a) { return vid(mbedtls_mpi_div_mpi(&mpi_, nullptr, &mpi_, a)); }
    mpi_t& operator%=(mpi_t const& a) { return vid(mbedtls_mpi_mod_mpi(&mpi_, &mpi_, a)); }


    // bitwise operations
    mpi_t& operator>>=(int  const& a) { return vid(mbedtls_mpi_shift_r(&mpi_, a)); }
    mpi_t& operator<<=(int  const& a) { return vid(mbedtls_mpi_shift_l(&mpi_, a)); }

    //
    mpi_t operator-() const { auto c = mpi_t(); vid(mbedtls_mpi_mul_int(c, &mpi_, -1)); return c; }

    // MPI
    mpi_t operator+(mpi_t const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_add_mpi(c, &mpi_, a)); return c; }
    mpi_t operator-(mpi_t const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_sub_mpi(c, &mpi_, a)); return c; }
    mpi_t operator*(mpi_t const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_mul_mpi(c, &mpi_, a)); return c; }
    mpi_t operator/(mpi_t const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_div_mpi(c, nullptr, &mpi_, a)); return c; }
    mpi_t operator%(mpi_t const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_mod_mpi(c, &mpi_, a)); return c; }

    // int
    mpi_t operator+(int const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_add_int(c, &mpi_, a)); return c; }
    mpi_t operator-(int const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_sub_int(c, &mpi_, a)); return c; }
    mpi_t operator*(int const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_mul_int(c, &mpi_, a)); return c; }
    mpi_t operator/(int const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_div_int(c, nullptr, &mpi_, a)); return c; }
    mbedtls_mpi_uint operator%(int const& a) const { auto c = mbedtls_mpi_uint(0); vid(mbedtls_mpi_mod_int(&c, &mpi_, a)); return c; }

    // uint
    mpi_t operator+(uint const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_add_int(c, &mpi_, a)); return c; }
    mpi_t operator-(uint const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_sub_int(c, &mpi_, a)); return c; }
    mpi_t operator*(uint const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_mul_int(c, &mpi_, a)); return c; }
    mpi_t operator/(uint const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_div_int(c, nullptr, &mpi_, a)); return c; }
    mbedtls_mpi_uint operator%(uint const& a) const { auto c = mbedtls_mpi_uint(0); vid(mbedtls_mpi_mod_int(&c, &mpi_, a)); return c; }

    // long
    mpi_t operator+(long const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_add_int(c, &mpi_, a)); return c; }
    mpi_t operator-(long const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_sub_int(c, &mpi_, a)); return c; }
    mpi_t operator*(long const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_mul_int(c, &mpi_, a)); return c; }
    mpi_t operator/(long const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_div_int(c, nullptr, &mpi_, a)); return c; }
    mbedtls_mpi_uint operator%(long const& a) const { auto c = mbedtls_mpi_uint(0); vid(mbedtls_mpi_mod_int(&c, &mpi_, a)); return c; }


    // bitwise operations
    //mpi_t operator>>(int  const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_shift_r(c, &mpi_, a)); return c; }
    //mpi_t operator<<(int  const& a) const { auto c = mpi_t(); vid(mbedtls_mpi_shift_l(c, &mpi_, a)); return c; }


    // setters
    mpi_t& set(int const& a) { return vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t& set(long const& a) { return vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t& set(uint const& a) { return vid(mbedtls_mpi_lset(&mpi_, a)); }
    mpi_t& set(mbedtls_mpi const& a) { return vid(mbedtls_mpi_copy(&mpi_, &a)); }
    mpi_t& set(mbedtls_mpi const* a) { return vid(mbedtls_mpi_copy(&mpi_, a)); }
    mpi_t& set(mpi_t const& a) { return vid(mbedtls_mpi_copy(&mpi_, a)); }


    // may be doupt
    mbedtls_mpi* operator&() { return &mpi_; }
    mbedtls_mpi const* operator&() const { return &mpi_; }

    //
    mbedtls_mpi* get() { return &mpi_; }
    mbedtls_mpi const* get() const { return &mpi_; }


    //
    operator mbedtls_mpi*() { return &mpi_; };
    operator mbedtls_mpi&() { return  mpi_; };
    operator mbedtls_mpi const*() const { return &mpi_; };
    operator mbedtls_mpi const&() const { return  mpi_; };


    //
    mpi_t& zero() { return vid(mbedtls_mpi_lset(&mpi_, 0), "invalid value"); }
    mpi_t& vid (auto const& status, std::string const& message = "invalid operation") {
        checkMbedtlsError(status, message);
        return *this;
    }

    mpi_t const& vid (auto const& status, std::string const& message = "invalid operation") const {
        checkMbedtlsError(status, message);
        return *this;
    }



    // TODO! use dedicated RAII object of random generation engine
    mpi_t& random() {
        // init random generator engine
        const char* pers = "spake2p_random";
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_context entropy;
        mbedtls_entropy_init(&entropy);
        int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char*>(pers), strlen(pers));
        if (ret != 0) { throw std::runtime_error("Ошибка инициализации генератора случайных чисел"); }

        // generation random itself
        ret = mbedtls_mpi_fill_random(&mpi_, 32, mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret != 0) { throw std::runtime_error("Ошибка генерации случайного числа"); }


        // free random gen memory
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        return *this;
    }


private:
    mbedtls_mpi mpi_;
};

#endif

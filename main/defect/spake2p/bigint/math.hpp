#pragma once
#include "../std/types.hpp"
#include "./intx.hpp"
#include "../../std/hex.hpp"

//
using bigint_t = intx::uint<256>;

//
namespace bmath {

    //
    inline void zeroize(void* d, size_t n) {
        volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(d);
        while (n--) *p++ = 0;
    }

    //
    inline bigint_t mod(
        bigint_t const& dividend, 
        bigint_t const& divisor
    ) {
        return (dividend % divisor);
        //bigint_t r = dividend % divisor;
        //return (r >= 0) ? r : r + divisor;
    }

    //
    inline bigint_t curve(bigint_t const& x, bigint_t const& B, bigint_t const& P) {
        return mod(mod(x * x, P) * x + B, P);
    };

    //
    inline bigint_t inv(bigint_t const& num, bigint_t const& md) {
        if (num == 0 || md <= 0) throw std::runtime_error("no inverse");
        bigint_t a = mod(num, md), b = md;
        bigint_t x = 0, y = 1, u = 1, v = 0;
        while (a != 0) {
            bigint_t q = b / a;
            bigint_t r = b % a;
            bigint_t m = x - u * q;
            bigint_t n = y - v * q;
            b = a; a = r; x = u; y = v; u = m; v = n;
        }
        if (b == 1) return mod(x, md);
        throw std::runtime_error("no inverse");
    }
};

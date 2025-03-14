#ifndef BD88A99E_90E2_4652_B277_35E5B254A4D4
#define BD88A99E_90E2_4652_B277_35E5B254A4D4

//
#include <std/types.hpp>
#include <std/hex.hpp>
#include "./mpi.hpp"

//
namespace mpc {
    // convert to uint256_t (bigint_t)
    bigint_t m2n(mbedtls_mpi const& mpi) {
        bigint_t x;
        mbedtls_mpi_write_binary(&mpi, (uint8_t*)&x, sizeof(x));
        return std::move(x);
    }

    // create by RAII
    mpi_t n2m(bigint_t const& x) {
        mpi_t mpi; mbedtls_mpi_read_binary(mpi, (uint8_t*)&x, sizeof(x)); return std::move(mpi);
    }

    // use existing
    mbedtls_mpi& n2m(bigint_t const& x, mbedtls_mpi& mpi) {
        mbedtls_mpi_read_binary(&mpi, (uint8_t*)&x, sizeof(x)); return mpi;
    }

    //
    std::string n2h(bigint_t const& I) {
        auto tmp = make_bytes(sizeof(I));
        memcpy(tmp->data(), &I, sizeof(I));
        return hex::b2h(tmp);
    }
}

//
#endif

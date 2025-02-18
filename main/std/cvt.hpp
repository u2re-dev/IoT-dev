#ifndef BD88A99E_90E2_4652_B277_35E5B254A4D4
#define BD88A99E_90E2_4652_B277_35E5B254A4D4

//
#include "./types.hpp"
#include "../spake2p/raii/mpi.hpp"

//
namespace mpc {
    // convert to uint256_t (bigint_t)
    bigint_t m2n(mbedtls_mpi const& mpi) {
        bigint_t x;
        mbedtls_mpi_write_binary_le(&mpi, (uint8_t*)&x, sizeof(x));
        return std::move(x);
    }

    // create by RAII
    mpi_t n2m(bigint_t const& x) {
        mpi_t mpi; mbedtls_mpi_read_binary_le(mpi, (uint8_t*)&x, sizeof(x)); return std::move(mpi);
    }

    // use existing
    mbedtls_mpi& n2m(bigint_t const& x, mbedtls_mpi& mpi) {
        mbedtls_mpi_read_binary_le(&mpi, (uint8_t*)&x, sizeof(x)); return mpi;
    }
}

#endif /* BD88A99E_90E2_4652_B277_35E5B254A4D4 */

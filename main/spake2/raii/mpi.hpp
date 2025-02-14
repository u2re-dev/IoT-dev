#ifndef C1AC2A76_36C0_483F_8C2E_21C70BA5DB80
#define C1AC2A76_36C0_483F_8C2E_21C70BA5DB80

//
#include "../crypto.hpp"
#include <mbedtls/ctr_drbg.h>
#include "../../../mbedtls/ecc_point_compression.h"

//
class MpiWrapper {
    public:
        MpiWrapper() { mbedtls_mpi_init(&mpi_); }
        ~MpiWrapper() { mbedtls_mpi_free(&mpi_); }
    
        mbedtls_mpi* get() { return &mpi_; }
        const mbedtls_mpi* get() const { return &mpi_; }
    
    private:
        mbedtls_mpi mpi_;
    };

#endif /* C1AC2A76_36C0_483F_8C2E_21C70BA5DB80 */

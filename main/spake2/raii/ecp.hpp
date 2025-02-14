#ifndef F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0
#define F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0

//
#include "../crypto.hpp"
#include <mbedtls/ctr_drbg.h>
#include "../../../mbedtls/ecc_point_compression.h"

//
class EcpPointWrapper {
public:
    EcpPointWrapper(const mbedtls_ecp_group& group) : group_(group) {
        mbedtls_ecp_point_init(&point_);
    }
    ~EcpPointWrapper() {
        mbedtls_ecp_point_free(&point_);
    }

    mbedtls_ecp_point* get() { return &point_; }
    const mbedtls_ecp_point* get() const { return &point_; }

    void setZero() {
        mbedtls_ecp_set_zero(&point_);
    }

private:
    const mbedtls_ecp_group& group_;
    mbedtls_ecp_point point_;
};


#endif /* F7D5C7A6_E633_4E9B_A6BD_D4AADE7F41E0 */

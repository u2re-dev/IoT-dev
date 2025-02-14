#ifndef C62DECF5_6D6A_4958_93B8_383B9AB3753A
#define C62DECF5_6D6A_4958_93B8_383B9AB3753A

//
#include "../crypto.hpp"
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include "../../../mbedtls/ecc_point_compression.h"

//
void checkMbedtlsError(int ret, const std::string& message) {
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        throw std::runtime_error(message + ": " + std::string(error_buf));
    }
}

#endif /* C62DECF5_6D6A_4958_93B8_383B9AB3753A */

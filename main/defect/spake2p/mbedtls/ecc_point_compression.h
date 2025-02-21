
#include <string.h>

#include "mbedtls/ecp.h"

//
#ifdef __cplusplus
extern "C" {
#endif

    int mbedtls_ecp_decompress(
        const mbedtls_ecp_group *grp,
        const unsigned char *input, size_t ilen,
        unsigned char *output, size_t *olen, size_t osize
    );

    int mbedtls_ecp_compress(
        const mbedtls_ecp_group *grp,
        const unsigned char *input, size_t ilen,
        unsigned char *output, size_t *olen, size_t osize
    );

#ifdef __cplusplus
}
#endif

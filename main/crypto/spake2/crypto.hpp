#pragma once

//
#include "../../std/types.hpp"

//
namespace crypto {
    bytes_t pbkdf2(const bytes_t& input, const bytes_t& salt, uint32_t iterations, size_t outputLength);
    bytes_t hkdf(const bytes_t& ikm, const bytes_t& salt, const bytes_t& info, size_t L);
    bytes_t hmac(const bytes_t& key, const bytes_t& data);
    bytes_t hash(const bytes_t& data);
    uint256_t getRandomBigint(size_t numbytes_t, const uint256_t& order);
}

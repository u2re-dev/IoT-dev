#pragma once

//
namespace crypto {
    ByteArray pbkdf2(const ByteArray& input, const ByteArray& salt, uint32_t iterations, size_t outputLength);
    ByteArray hkdf(const ByteArray& ikm, const ByteArray& salt, const ByteArray& info, size_t L);
    ByteArray hmac(const ByteArray& key, const ByteArray& data);
    ByteArray hash(const ByteArray& data);
    uint256_t getRandomBigInt(size_t numBytes, const uint256_t& order);
}

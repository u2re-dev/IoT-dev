#pragma once
#include "./std.hpp"

//
constexpr uint8_t  H_VERSION   = 0x01;
constexpr uint16_t SER_VERSION = 0x0001;

//
constexpr size_t CRYPTO_GROUP_SIZE_BYTES = 32;
constexpr size_t CRYPTO_W_SIZE_BYTES = CRYPTO_GROUP_SIZE_BYTES + 8;
constexpr size_t PBKDF2_OUTLEN = CRYPTO_W_SIZE_BYTES * 2;

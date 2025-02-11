#pragma once
#include "../std/utils.hpp"

//
constexpr uint8_t  H_VERSION   = 0x01;
constexpr uint16_t SER_VERSION = 0x0001;

//
constexpr size_t crypto_spake_DUMMYKEYBYTES    = 32;
constexpr size_t crypto_spake_PUBLICDATABYTES  = 36;
constexpr size_t crypto_spake_RESPONSE1BYTES   = 32;
constexpr size_t crypto_spake_RESPONSE2BYTES   = 64;
constexpr size_t crypto_spake_RESPONSE3BYTES   = 32;
constexpr size_t crypto_spake_SHAREDKEYBYTES   = 32;
constexpr size_t crypto_spake_STOREDBYTES      = 164;

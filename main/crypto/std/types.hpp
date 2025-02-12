#pragma once

//
#include <array>
#include <stdexcept>
#include <sstream>
#include <concepts>
#include <string>
#include <vector>
#include <iomanip>

//
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdint>

//
using BigInt    = uint256_t;
using ByteArray = std::vector<uint8_t>;
using Bytes     = ByteArray;

//
struct AffinePoint { BigInt x; BigInt y; };

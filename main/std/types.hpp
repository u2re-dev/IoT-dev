#pragma once

//
#include <array>
#include <stdexcept>
#include <sstream>
#include <concepts>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <utility>
#include <functional>
#include <atomic>
#include <thread>

//
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdint>

//
#include "./uint128/uint128_t.h"
#include "./uint256/uint256_t.h"

//
using bigint_t = uint256_t;
using bytes_t  = std::vector<uint8_t>;

//
struct affine_t { bigint_t x; bigint_t y; };

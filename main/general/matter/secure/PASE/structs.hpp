#ifndef AA6D0DF3_9920_4AC8_93DD_C8EB9664B999
#define AA6D0DF3_9920_4AC8_93DD_C8EB9664B999

//
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>

//
#include <std/types.hpp>

//
struct PBKDFParamRequest { bigint_t rand = 0; uint16_t sess = 0; uint8_t pass = 0; };
struct SessionKeys { intx::uint128 I2Rkeys = 0, R2Ikeys = 0, AttestationChallenge = 0; };

//
struct RNG {
    std::random_device dev;
    std::mt19937 rng;

    //
    RNG() : rng(std::mt19937(dev())) {};
    RNG(RNG const& o) : rng(o.rng) {};

    //
    template<typename T = uint64_t> T generate() {
        std::uniform_int_distribution<T> dist;
        return dist(rng);
    }
};

//
#endif

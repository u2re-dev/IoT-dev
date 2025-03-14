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

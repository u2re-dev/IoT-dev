#pragma once
#include "./std.hpp"

//
struct SpakeKeys {
    std::array<unsigned char, 32> M;
    std::array<unsigned char, 32> N;
    std::array<unsigned char, 32> L;
    std::array<unsigned char, 32> h_K;
    std::array<unsigned char, 32> h_L;
};

struct SpakeValidators {
    std::array<unsigned char, 32> client_validator;
    std::array<unsigned char, 32> server_validator;
};

struct SharedKeys {
    std::array<unsigned char, crypto_spake_SHAREDKEYBYTES> client_sk;
    std::array<unsigned char, crypto_spake_SHAREDKEYBYTES> server_sk;
};

struct ClientState {
    std::array<unsigned char, 32> h_K;
    std::array<unsigned char, 32> h_L;
    std::array<unsigned char, 32> N;
    std::array<unsigned char, 32> x; // secret scalar
    std::array<unsigned char, 32> X; // public
};

struct ServerState {
    SharedKeys shared_keys;
    std::array<unsigned char, 32> server_validator;
};

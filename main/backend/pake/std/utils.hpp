#pragma once
#include <cassert>
#include <cstring>
#include <array>
#include <iostream>

//
static void push16(unsigned char *out, size_t &pos, uint16_t v) {
    out[pos++] = (unsigned char)(v       & 0xFF);
    out[pos++] = (unsigned char)(v >> 8  & 0xFF);
}

static void push64(unsigned char *out, size_t &pos, uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        out[pos++] = (unsigned char)(v >> (8*i));
    }
}

static void push128(unsigned char *out, size_t &pos, const unsigned char v[16]) {
    std::memcpy(out + pos, v, 16);
    pos += 16;
}

static void push256(unsigned char *out, size_t &pos, const unsigned char v[32]) {
    std::memcpy(out + pos, v, 32);
    pos += 32;
}

static void pop16(uint16_t &v, const unsigned char *in, size_t &pos) {
    v = ((uint16_t) in[pos]) | (((uint16_t) in[pos+1]) << 8);
    pos += 2;
}

static void pop64(uint64_t &v, const unsigned char *in, size_t &pos) {
    v = 0;
    for (int i = 0; i < 8; ++i)
        v |= ((uint64_t) in[pos+i]) << (8*i);
    pos += 8;
}

static void pop128(unsigned char v[16], const unsigned char *in, size_t &pos) {
    std::memcpy(v, in+pos, 16);
    pos += 16;
}

static void pop256(unsigned char v[32], const unsigned char *in, size_t &pos) {
    std::memcpy(v, in+pos, 32);
    pos += 32;
}

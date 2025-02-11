#include "blake2b.hpp"
#include <cstring>
#include <stdexcept> // For std::invalid_argument


const std::array<uint64_t, 8> BLAKE2b::IV = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
    0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

const std::array<std::array<uint8_t, 16>, 12> BLAKE2b::SIGMA ={{
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3}
}};

BLAKE2b::BLAKE2b(uint8_t cbHashLen, const std::vector<uint8_t>& Key) : cbHashLen(cbHashLen) {
    if (cbHashLen == 0 || cbHashLen > 64) {
        throw std::invalid_argument("Invalid hash length specified for BLAKE2b.");
    }
    initialize(Key);
}

std::vector<uint8_t> BLAKE2b::hash(const std::vector<uint8_t>& M) {
    std::vector<uint8_t> out(cbHashLen);
    // Reset state for new hash computation
    initialize(std::vector<uint8_t>());
    update(M.data(), M.size());
    finalize(out);
    return out;
}

void BLAKE2b::initialize(const std::vector<uint8_t>& Key) {
    h = IV; // Copy IV to internal state
    h[0] ^= 0x01010000 ^ (static_cast<uint64_t>(Key.size()) << 8) ^ cbHashLen;
    if (!Key.empty()) {
        std::vector<uint8_t> block(128, 0);
        std::memcpy(block.data(), Key.data(), Key.size());
        update(block.data(), block.size());
    }
}

// In BLAKE2b.cpp
void BLAKE2b::compress(const uint8_t* block, bool isLastBlock) {
    uint64_t m[16];
    uint64_t v[16];

    // Load the message block into 16 64-bit words (little-endian)
    for (size_t i = 0; i < 16; ++i) {
        m[i] = 0;
        for (size_t j = 0; j < 8; ++j) {
            m[i] |= static_cast<uint64_t>(block[i * 8 + j]) << (j * 8);
        }
    }

    // Initialize state vector v
    for (size_t i = 0; i < 8; ++i) {
        v[i] = h[i]; // First half from the current state
        v[i + 8] = IV[i]; // Second half from IV
    }

    // Low 64 bits of the offset
    v[12] ^= t_low;
    // High 64 bits of the offset
    v[13] ^= t_high;

    if (isLastBlock) {
        // Invert all bits if this is the last block
        v[14] = ~v[14];
    }

    // 12 rounds of mixing
    for (size_t i = 0; i < 12; i++) {
        // Mix columns
        mix(v[0], v[4], v[8],  v[12], m[SIGMA[i][0]], m[SIGMA[i][1]]);
        mix(v[1], v[5], v[9],  v[13], m[SIGMA[i][2]], m[SIGMA[i][3]]);
        mix(v[2], v[6], v[10], v[14], m[SIGMA[i][4]], m[SIGMA[i][5]]);
        mix(v[3], v[7], v[11], v[15], m[SIGMA[i][6]], m[SIGMA[i][7]]);
        // Mix diagonals
        mix(v[0], v[5], v[10], v[15], m[SIGMA[i][8]],  m[SIGMA[i][9]]);
        mix(v[1], v[6], v[11], v[12], m[SIGMA[i][10]], m[SIGMA[i][11]]);
        mix(v[2], v[7], v[8],  v[13], m[SIGMA[i][12]], m[SIGMA[i][13]]);
        mix(v[3], v[4], v[9],  v[14], m[SIGMA[i][14]], m[SIGMA[i][15]]);
    }

    // Update the state with the result of the transformation
    for (size_t i = 0; i < 8; ++i) {
        h[i] ^= v[i] ^ v[i + 8];
    }
}

void BLAKE2b::update(const uint8_t* data, size_t len) {
    size_t remaining = len;
    size_t offset = 0;

    // If there's leftover data from a previous update, process it first
    if (buffer.size() > 0) {
        size_t toFill = 128 - buffer.size();
        if (remaining >= toFill) {
            buffer.insert(buffer.end(), data, data + toFill);
            compress(buffer.data(), false);
            buffer.clear();
            offset += toFill;
            remaining -= toFill;
        } else {
            buffer.insert(buffer.end(), data, data + remaining);
            return;
        }
    }

    // Process all 128-byte blocks
    while (remaining >= 128) {
        compress(data + offset, false);
        offset += 128;
        remaining -= 128;
    }

    // Store leftover data
    if (remaining > 0) {
        buffer.insert(buffer.end(), data + offset, data + offset + remaining);
    }
}


void BLAKE2b::finalize(std::vector<uint8_t>& out) {
    // Pad any remaining data to 128 bytes
    if (buffer.size() < 128) {
        buffer.resize(128, 0);
    }

    // Set the last block flag
    compress(buffer.data(), true);

    // Extract the hash value
    for (size_t i = 0; i < h.size() && (i * 8) < out.size(); ++i) {
        for (size_t j = 0; j < 8 && (i * 8 + j) < out.size(); ++j) {
            out[i * 8 + j] = (h[i] >> (8 * j)) & 0xFF;
        }
    }

    // Clear temporary buffers for security
    std::fill(buffer.begin(), buffer.end(), 0);
}

inline uint64_t BLAKE2b::rotr64(uint64_t x, uint8_t r) {
    return (x >> r) | (x << (64 - r));
}

void BLAKE2b::mix(uint64_t& a, uint64_t& b, uint64_t& c, uint64_t& d, uint64_t x, uint64_t y) {
    a = a + b + x;
    d = rotr64(d ^ a, 32);
    c = c + d;
    b = rotr64(b ^ c, 24);
    a = a + b + y;
    d = rotr64(d ^ a, 16);
    c = c + d;
    b = rotr64(b ^ c, 63);
}

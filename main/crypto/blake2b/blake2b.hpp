#ifndef BLAKE2B_HPP
#define BLAKE2B_HPP

//
#ifdef ENABLE_BLAKDE2B
#include <array>
#include <vector>
#include <cstdint>

/**
 * @class BLAKE2b
 * @brief Implementation of the BLAKE2b cryptographic hash function.
 *
 * BLAKE2b is an optimized cryptographic hash function designed for
 * both speed and security, making it suitable for a wide range of applications.
 * This class provides an interface to compute BLAKE2b hashes for input messages
 * with optional keying support for keyed hashing (HMAC).
 */
class BLAKE2b {
public:
    /**
     * @brief Constructs a BLAKE2b hash object with optional hash length and key.
     *
     * @param cbHashLen Desired length of the hash output in bytes. Default is 64 bytes.
     * @param Key Optional key for keyed hashing. Empty vector means no key is used.
     */
    BLAKE2b(uint8_t cbHashLen = 64, const std::vector<uint8_t>& Key = {});

    /**
     * @brief Computes the BLAKE2b hash of the input message.
     *
     * @param M The input message to hash.
     * @return A vector containing the hash of the input message.
     */
    std::vector<uint8_t> hash(const std::vector<uint8_t>& M);

private:
    static const std::array<uint64_t, 8> IV;
    static const std::array<std::array<uint8_t, 16>, 12> SIGMA;


    std::array<uint64_t, 8> h; ///< Internal state.
    uint64_t t_low = 0, t_high = 0; ///< Counter (t) for the number of bytes hashed.
    uint8_t cbHashLen; ///< Desired length of the hash output.
    std::vector<uint8_t> buffer;

    /**
     * @brief Initializes the hash state with the optional key.
     *
     * @param Key The key used for keyed hashing. Empty vector means no key is used.
     */
    void initialize(const std::vector<uint8_t>& Key);

    /**
     * @brief Compresses a single 128-byte block of the message.
     *
     * @param block Pointer to the 128-byte block to compress.
     * @param isLastBlock Indicates if this is the last block of the message.
     */
    void compress(const uint8_t* block, bool isLastBlock);

    /**
     * @brief Updates the hash state with a portion of the message.
     *
     * @param data Pointer to the message data to hash.
     * @param len Length of the message data in bytes.
     */
    void update(const uint8_t* data, size_t len);

    /**
     * @brief Finalizes the hash computation and produces the hash output.
     *
     * @param out Reference to a vector where the hash output will be stored.
     */
    void finalize(std::vector<uint8_t>& out);

    // Helper functions
    static inline uint64_t rotr64(uint64_t x, uint8_t r); ///< Performs right rotation on a 64-bit word.
    static void mix(uint64_t& a, uint64_t& b, uint64_t& c, uint64_t& d, uint64_t x, uint64_t y); ///< BLAKE2b mixing function.
};

// Definition of the IV constants

#endif
#endif // BLAKE2B_HPP

#ifndef ECDSA_HPP
#define ECDSA_HPP

#include "ecdh.hpp"
#include "../../blake2b/blake2b.hpp"
#include <utility>

/**
 * @brief Converts a vector of bytes (uint8_t) to a hexadecimal string.
 *
 * This function takes a vector of bytes (each byte being an unsigned 8-bit integer)
 * and converts it into a string representation where each byte is represented
 * by its hexadecimal equivalent. For example, a byte `0xB4` would be represented
 * as the string "B4". This is useful for displaying or storing binary data in
 * a human-readable hexadecimal format.
 *
 * @param bytes The vector of bytes to be converted to a hexadecimal string.
 * @return A std::string representing the hexadecimal value of the input bytes.
 *
 * Example Usage:
 * @code
 * std::vector<uint8_t> data = {0xBA, 0xAD, 0xF0, 0x0D};
 * std::string hexStr = bytesToHex(data);
 * // hexStr will be "BAADF00D"
 * @endcode
 */
std::string bytesToHex(const std::vector<uint8_t>& bytes);


/**
 * @class ECDSA
 * @brief Implementation of Elliptic Curve Digital Signature Algorithm (ECDSA).
 *
 * ECDSA provides functionalities to sign and verify messages using elliptic curve cryptography,
 * ensuring data integrity and authenticity.
 */
class ECDSA {
public:
    /**
     * @brief Signs a given message using ECDSA with a private key.
     *
     * This function generates a digital signature consisting of a pair of BigInt values
     * (r, s) using the provided private key and the message. The message is first hashed
     * using a secure hash algorithm before signing.
     *
     * @param privateKey The private key used for signing.
     * @param message The message to be signed.
     * @return A pair of BigInts (r, s) representing the signature.
     * Example Usage:
     * @code
     * BigInt privateKey = ;
     * std::string msg = "Example message";
     * std::vector<uint8_t> message(msg.begin(), msg.end());
     * auto signature = ECDSA::sign(privateKey, message);
     * @endcode
     * // Use the signature as needed...
     */
    static std::pair<BigInt, BigInt> sign(const BigInt& privateKey, const std::vector<uint8_t>& message);

    /**
     * @brief Verifies a given ECDSA signature against a message using the public key.
     *
     * This function verifies the authenticity of a signature by using the public key
     * corresponding to the private key used for signing. The function returns true if
     * the signature is valid and false otherwise.
     *
     * @param publicKey The public key associated with the private key used for signing.
     * @param message The original message that was signed.
     * @param signature A pair of BigInts (r, s) representing the signature.
     * @return True if the signature is valid, false otherwise.
     *
     * @example
     * @code
     * Ecc_Point publicKey = ;
     * std::string msg = "Example message";
     * std::vector<uint8_t> message(msg.begin(), msg.end());
     * std::pair<BigInt, BigInt> signature = ;
     * bool isValid = ECDSA::verify(publicKey, message, signature);
     * if (isValid) {
     *     // Signature is valid
     * } else {
     *     // Signature is invalid
     * }
     * @endcode
     */
    static bool verify(const Ecc_Point& publicKey, const std::vector<uint8_t>& message, const std::pair<BigInt, BigInt>& signature);

private:
    // Additional private methods or member variables
};

#endif // ECDSA_HPP

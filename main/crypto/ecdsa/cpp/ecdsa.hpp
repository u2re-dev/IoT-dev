#ifndef ECDSA_HPP
#define ECDSA_HPP

//
#ifdef ENABLE_ECDSA
#include "ecdh.hpp"
#include "../../blake2b/blake2b.hpp"
#include <utility>

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
     * This function generates a digital signature consisting of a pair of bigint_t values
     * (r, s) using the provided private key and the message. The message is first hashed
     * using a secure hash algorithm before signing.
     *
     * @param privateKey The private key used for signing.
     * @param message The message to be signed.
     * @return A pair of bigint_ts (r, s) representing the signature.
     * Example Usage:
     * @code
     * bigint_t privateKey = ;
     * std::string msg = "Example message";
     * std::vector<uint8_t> message(msg.begin(), msg.end());
     * auto signature = ECDSA::sign(privateKey, message);
     * @endcode
     * // Use the signature as needed...
     */
    static std::pair<bigint_t, bigint_t> sign(const bigint_t& privateKey, const std::vector<uint8_t>& message);

    /**
     * @brief Verifies a given ECDSA signature against a message using the public key.
     *
     * This function verifies the authenticity of a signature by using the public key
     * corresponding to the private key used for signing. The function returns true if
     * the signature is valid and false otherwise.
     *
     * @param publicKey The public key associated with the private key used for signing.
     * @param message The original message that was signed.
     * @param signature A pair of bigint_ts (r, s) representing the signature.
     * @return True if the signature is valid, false otherwise.
     *
     * @example
     * @code
     * eccp_t publicKey = ;
     * std::string msg = "Example message";
     * std::vector<uint8_t> message(msg.begin(), msg.end());
     * std::pair<bigint_t, bigint_t> signature = ;
     * bool isValid = ECDSA::verify(publicKey, message, signature);
     * if (isValid) {
     *     // Signature is valid
     * } else {
     *     // Signature is invalid
     * }
     * @endcode
     */
    static bool verify(const eccp_t& publicKey, const std::vector<uint8_t>& message, const std::pair<bigint_t, bigint_t>& signature);

private:
    // Additional private methods or member variables
};

#endif
#endif // ECDSA_HPP

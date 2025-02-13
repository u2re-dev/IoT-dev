#ifndef ECDH_HPP
#define ECDH_HPP

//
#ifdef ENABLE_ECDH
#include "../../ecc/ecc.hpp"
//#include "bigint.hpp"

/**
 * @file ecdh.hpp
 * @brief Elliptic Curve Diffie-Hellman (ECDH) Key Exchange
 */

/**
 * @namespace ECDH
 * @brief Namespace for Elliptic Curve Diffie-Hellman Key Exchange functions
 */
namespace ECDH {

    /**
     * @brief Generates an ECC key pair for ECDH key exchange.
     *
     * This function generates a public-private key pair to be used in the ECDH
     * key exchange process. The private key is a randomly generated bigint_t,
     * and the public key is an eccp_t on the elliptic curve, computed as
     * the scalar multiplication of the base point by the private key.
     *
     * @return A pair consisting of an eccp_t (public key) and a bigint_t (private key).
     */
    std::pair<eccp_t, bigint_t> generateKeyPair();

    /**
     * @brief Computes the shared secret using ECDH key exchange.
     *
     * Given a private key and the other party's public key, this function computes
     * the shared secret by performing scalar multiplication of the other party's
     * public key by the private key. The resulting eccp_t is the shared secret,
     * which can then be used to derive a symmetric key for encryption.
     *
     * @param privateKey The private key (bigint_t) of the calling party.
     * @param otherPublicKey The public key (eccp_t) of the other party.
     * @return The shared secret as an eccp_t, resulting from the ECDH key exchange.
     */
    eccp_t computeSharedSecret(const bigint_t& privateKey, const eccp_t& otherPublicKey);

} // namespace ECDH

#endif
#endif // ECDH_HPP

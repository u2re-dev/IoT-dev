#ifndef ECDH_HPP
#define ECDH_HPP

//
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
     * key exchange process. The private key is a randomly generated BigInt,
     * and the public key is an Ecc_Point on the elliptic curve, computed as
     * the scalar multiplication of the base point by the private key.
     *
     * @return A pair consisting of an Ecc_Point (public key) and a BigInt (private key).
     */
    std::pair<Ecc_Point, BigInt> generateKeyPair();

    /**
     * @brief Computes the shared secret using ECDH key exchange.
     *
     * Given a private key and the other party's public key, this function computes
     * the shared secret by performing scalar multiplication of the other party's
     * public key by the private key. The resulting Ecc_Point is the shared secret,
     * which can then be used to derive a symmetric key for encryption.
     *
     * @param privateKey The private key (BigInt) of the calling party.
     * @param otherPublicKey The public key (Ecc_Point) of the other party.
     * @return The shared secret as an Ecc_Point, resulting from the ECDH key exchange.
     */
    Ecc_Point computeSharedSecret(const BigInt& privateKey, const Ecc_Point& otherPublicKey);

} // namespace ECDH

#endif // ECDH_HPP

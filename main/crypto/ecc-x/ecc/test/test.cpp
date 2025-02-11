#include "ecdh.hpp"
#include "blake2b.hpp"
#include "ecdsa.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

void ecdh_test() {
    // Generate key pair for Party A
    auto [publicKeyA, privateKeyA] = ECDH::generateKeyPair();
    std::cout << "Party A Public Key: ";
    publicKeyA.print();  // Print Party A's public key

    // Generate key pair for Party B
    auto [publicKeyB, privateKeyB] = ECDH::generateKeyPair();
    std::cout << "Party B Public Key: ";
    publicKeyB.print();  // Print Party B's public key

    // Compute shared secrets
    Ecc_Point sharedSecretA = ECDH::computeSharedSecret(privateKeyA, publicKeyB);
    Ecc_Point sharedSecretB = ECDH::computeSharedSecret(privateKeyB, publicKeyA);

    // Print shared secrets to verify they are the same
    std::cout << "Shared Secret (Computed by Party A): ";
    sharedSecretA.print();  // Print the shared secret as computed by Party A
    std::cout << "Shared Secret (Computed by Party B): ";
    sharedSecretB.print();  // Print the shared secret as computed by Party B

    // Optionally, compare the shared secrets to ensure they match
    if (sharedSecretA == sharedSecretB) {
        std::cout << "ECDH Key Exchange successful. Shared secrets match." << std::endl;
    } else {
        std::cout << "ECDH Key Exchange failed. Shared secrets do not match." << std::endl;
    }
}

void blake_2b_test() {
    BLAKE2b blake2b(64);

    // Test strings
    std::string input1 = "Cody Rodes!";
    std::string input2 = "Dwane the Rock Johnson!";

    // Compute hashes
    auto hash1 = blake2b.hash(std::vector<uint8_t>(input1.begin(), input1.end()));
    auto hash2 = blake2b.hash(std::vector<uint8_t>(input2.begin(), input2.end()));

    // Convert hashes to hex strings
    std::string hashStr1 = bytesToHex(hash1);
    std::string hashStr2 = bytesToHex(hash2);

    std::cout << "Hash 1: " << hashStr1 << std::endl;

    std::cout << "Hash 2: " << hashStr2 << std::endl;
}

void test_ecdsa(){
    auto [publicKey, privateKey]=ECDH::generateKeyPair();

    // Message to be signed
    std::string messageStr = "Hello, world!";
    std::vector<uint8_t> message(messageStr.begin(), messageStr.end());

    // Signing the message
    auto signature = ECDSA::sign(privateKey, message);
    std::cout << "Signature: (r, s) = (" << signature.first.toString() << ", " << signature.second.toString() << ")" << std::endl;

    // Verifying the signature
    bool isValid = ECDSA::verify(publicKey, message, signature);
    std::cout << "Signature is " << (isValid ? "valid" : "invalid") << std::endl;
}

int main(){
    // ecdh_test();
    // blake_2b_test();
    test_ecdsa();
    return 0;
}
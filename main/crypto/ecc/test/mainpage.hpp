/*! \mainpage ecc-cpp Documentation
 *
 * \section intro_sec Introduction
 *
 * Welcome to the ecc-cpp documentation. This project provides a comprehensive C++ library that implements Elliptic Curve Cryptography (ECC).
 * It includes functionalities for basic elliptic curve operations, Elliptic Curve Diffie-Hellman (ECDH), and Elliptic Curve Digital Signature Algorithm (ECDSA),
 * alongside an implementation of the Blake2b cryptographic hash function.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Clone the repository
 * \code{.sh}
 * git clone https://github.com/crypto-keys-unlocked/ecc-cpp.git
 * \endcode
 *
 * \subsection step2 Step 2: Build the project
 * Navigate to the project directory and build the project:
 * \code{.sh}
 * mkdir build && cd build
 * cmake ..
 * make
 * \endcode
 *
 * \section usage_sec Usage
 *
 * Include the desired headers from the `include/` directory in your project and link against the `ecc-cpp` library.
 * Here's an example that demonstrates signing and verifying a message using ECDSA:
 *
 * \code{.cpp}
 * #include "ecdsa.hpp"
 * #include "ecc.hpp"
 * #include "bigint.hpp"
 *
 * int main() {
 *     auto [publicKey, privateKey] = ECDH::generateKeyPair();
 *     std::string message = "Hello, world!";
 *     auto signature = ECDSA::sign(privateKey, message);
 *     bool isValid = ECDSA::verify(publicKey, message, signature);
 *     
 *     if (isValid) {
 *         std::cout << "Signature is valid." << std::endl;
 *     } else {
 *         std::cout << "Signature is invalid." << std::endl;
 *     }
 *
 *     return 0;
 * }
 * \endcode
 *
 * \section contribute_sec Contributing
 *
 * Contributions are what make the open-source community such an amazing place to learn, inspire, and create.
 * Any contributions you make are **greatly appreciated**.
 *
 * If you have a suggestion that would make this better, please fork the repo and create a pull request.
 * You can also simply open an issue with the tag "enhancement".
 * Don't forget to give the project a star! Thanks again!
 *
 * \section license_sec License
 *
 * Distributed under the MIT License. See `LICENSE.md` for more information.
 *
 * \section contact_sec Contact
 *
 * Debrup Chatterjee - debrupc3@gmail.com
 * Project Link: https://github.com/crypto-keys-unlocked/ecc-cpp.git
 */

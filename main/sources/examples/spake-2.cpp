#ifdef USE_OPENSSL
int main() {
    //
    OpenSSL_add_all_algorithms();

    //
    uint32_t pin = 123456;
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        std::cerr << "RAND_bytes failed\n";
        return 1;
    }
    int iterations = 10000;

    //
    BIGNUM* w0 = nullptr, w1 = nullptr;
    if (computeW0W1(pin, salt, iterations, &w0, &w1) != 0) {
        std::cerr << "computeW0W1 failed\n";
        return 1;
    }

    //
    char* w0_hex = BN_bn2hex(w0);
    std::cout << "w0: " << w0_hex << "\n";
    OPENSSL_free(w0_hex);

    //
    std::vector<unsigned char> protocol_context = {'S','P','A','K','E','2','P'};

    //
    Spake2p* spake = Spake2p::create(protocol_context, w0);
    if (!spake) { BN_free(w1); return 1; };

    //
    EC_POINT* X = spake->computeX();
    EC_POINT* Y = spake->computeY();

    //
    std::vector<unsigned char> Ke, hAY, hBX;
    if (!spake->computeSecretAndVerifiersFromY(w1, X, Y, Ke, hAY, hBX)) {
        std::cerr << "computeSecretAndVerifiersFromY failed\n";
    } else {
        std::cout << "Ke: ";    for (auto b : Ke)  printf("%02x", b);
        std::cout << "\nhAY: "; for (auto b : hAY) printf("%02x", b);
        std::cout << "\nhBX: "; for (auto b : hBX) printf("%02x", b);
        std::cout << "\n";
    }

    //
    EC_POINT_free(X);
    EC_POINT_free(Y);
    BN_free(w1);
    delete spake;
    EVP_cleanup();
    return 0;
}
#endif

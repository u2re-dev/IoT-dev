int main() {
    uint32_t pin = 123456;
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        std::cerr << "RAND_bytes failed\n";
        return 1;
    }
    int iterations = 10000;

    //
    BIGNUM *w0 = nullptr, *w1 = nullptr;
    if (computeW0W1(pin, salt, iterations, &w0, &w1) != 0) {
        std::cerr << "computeW0W1 failed\n";
        return 1;
    }

    // show values
    char *w0_hex = BN_bn2hex(w0); std::cout << "w0: " << w0_hex << "\n";
    char *w1_hex = BN_bn2hex(w1); std::cout << "w1: " << w1_hex << "\n";

    //
    OPENSSL_free(w0_hex); BN_free(w0);
    OPENSSL_free(w1_hex); BN_free(w1);

    //
    return 0;
}

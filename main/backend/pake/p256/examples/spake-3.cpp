int main() {
    //
    uint32_t pin = 123456;
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        std::cerr << "RAND_bytes failed\n";
        return 1;
    }
    int iterations = 10000;

    //
    BIGNUM* w0_client = nullptr; BIGNUM* w1_client = nullptr;
    BIGNUM* w0_server = nullptr; BIGNUM* w1_server = nullptr;

    //
    if (computeW0W1(pin, salt, iterations, &w0_client, &w1_client) != 0) { std::cerr << "Client computeW0W1 failed\n"; return 1; }
    if (computeW0W1(pin, salt, iterations, &w0_server, &w1_server) != 0) { std::cerr << "Server computeW0W1 failed\n"; return 1; }

    //
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (!group) { std::cerr << "EC_GROUP_new_by_curve_name failed\n"; return 1; }

    //
    EC_POINT* L = Spake2p::computeL(group, w1_server);
    if (!L) { std::cerr << "computeL failed\n"; return 1; }

    //
    std::vector<unsigned char> protocolContext = {'S','P','A','K','E','2','P'};

    // ----- Compute X (client, pake1)
    Spake2p* spakeClient = Spake2p::create(protocolContext, w0_client);
    if (!spakeClient) { std::cerr << "spakeClient creation failed\n"; return 1; }
    EC_POINT* X = spakeClient->computeX();

    // ----- Compute Y (server, pake2)
    Spake2p* spakeServer = Spake2p::create(protocolContext, w0_server);
    if (!spakeServer) { std::cerr << "spakeServer creation failed\n"; return 1; }
    EC_POINT* Y = spakeServer->computeY();

    // ----- Compute for (server, pake3)
    std::vector<unsigned char> Ke_server, hAY_server, hBX_server;
    if (!spakeServer->computeSecretAndVerifiersFromX(L, X, Y, Ke_server, hAY_server, hBX_server)) {
        std::cerr << "spakeServer computeSecretAndVerifiersFromX failed\n";
        return 1;
    }

    // ----- Compute for (client, pake3)
    std::vector<unsigned char> Ke_client, hAY_client, hBX_client;
    if (!spakeClient->computeSecretAndVerifiersFromY(w1_client, X, Y, Ke_client, hAY_client, hBX_client)) {
        std::cerr << "spakeClient computeSecretAndVerifiersFromY failed\n";
        return 1;
    }

    //
    if (hBX_client != hBX_server) {
        std::cerr << "Клиент: неверный серверный верификатор\n";
        return 1;
    }

    //
    if (hAY_client != hAY_server) {
        std::cerr << "Сервер: неверный клиентский верификатор\n";
        return 1;
    }

    //
    std::cout << "PASE обмен завершён успешно!\n";

    //
    EC_POINT_free(X);
    EC_POINT_free(Y);
    EC_POINT_free(L);

    //
    delete spakeClient;
    delete spakeServer;

    //
    BN_free(w1_client);
    BN_free(w0_server);
    BN_free(w1_server);
    EVP_cleanup();

    //
    return 0;
}

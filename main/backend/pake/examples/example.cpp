
/*
//=================================================================
// Пример использования протокола SPAKE2+ (демонстрация обмена)
int main()
{
    if (sodium_init() < 0) {
        std::cerr << "Ошибка инициализации libsodium\n";
        return 1;
    }

    // Примерные параметры
    const char *password = "mysecretpassword";
    unsigned long long passwdlen = std::strlen(password);
    unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
    size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

    // Сервер генерирует stored_data и public_data
    unsigned char stored_data[crypto_spake_STOREDBYTES] = {0};
    if (crypto_spake_server_store(stored_data, password, passwdlen, opslimit, memlimit) != 0) {
        std::cerr << "Ошибка создания хранимых данных на сервере.\n";
        return 1;
    }
    unsigned char public_data[crypto_spake_PUBLICDATABYTES] = {0};
    ServerState server_state{};
    if (crypto_spake_step0(server_state, public_data, stored_data) != 0) {
        std::cerr << "Ошибка формирования public_data сервером.\n";
        return 1;
    }

    // Клиент получает public_data и вычисляет X (response1)
    ClientState client_state{};
    unsigned char response1[crypto_spake_RESPONSE1BYTES] = {0};
    if (crypto_spake_step1(client_state, response1, public_data, password, passwdlen) != 0) {
        std::cerr << "Клиент: ошибка на шаге 1.\n";
        return 1;
    }

    // Сервер получает X (response1) и вычисляет Y + client_validator (response2)
    unsigned char response2[crypto_spake_RESPONSE2BYTES] = {0};
    const char *client_id = "client@example.com";
    const char *server_id = "server@example.com";
    if (crypto_spake_step2(server_state, response2, client_id, std::strlen(client_id),
                           server_id, std::strlen(server_id),
                           stored_data, response1) != 0)
    {
        std::cerr << "Сервер: ошибка на шаге 2.\n";
        return 1;
    }

    // Клиент получает response2, проверяет client_validator и отправляет server_validator (response3)
    unsigned char response3[crypto_spake_RESPONSE3BYTES] = {0};
    SharedKeys client_shared_keys{};
    if (crypto_spake_step3(client_state, response3, client_shared_keys,
                           client_id, std::strlen(client_id),
                           server_id, std::strlen(server_id),
                           response2) != 0)
    {
        std::cerr << "Клиент: ошибка на шаге 3.\n";
        return 1;
    }

    // Сервер получает response3 и проверяет server_validator, в случае успеха получает общий разделённый ключ
    SharedKeys server_shared_keys{};
    if (crypto_spake_step4(server_state, server_shared_keys, response3) != 0) {
        std::cerr << "Сервер: ошибка на шаге 4.\n";
        return 1;
    }

    // Если протокол прошёл успешно, оба участника получили одинаковые ключи.
    if (sodium_memcmp(client_shared_keys.client_sk.data(),
                      server_shared_keys.client_sk.data(),
                      crypto_spake_SHAREDKEYBYTES) == 0)
    {
        std::cout << "SPAKE2+ успешно завершён, общий ключ совпадает.\n";
    } else {
        std::cerr << "Ошибка: ключи не совпадают!\n";
        return 1;
    }
    return 0;
}




int main() {
    // Пример: PIN, соль (16 байт) и число итераций
    uint32_t pin = 123456;  // пример PIN
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        std::cerr << "RAND_bytes failed\n";
        return 1;
    }
    int iterations = 10000;

    BIGNUM *w0 = nullptr, *w1 = nullptr;
    if (computeW0W1(pin, salt, iterations, &w0, &w1) != 0) {
        std::cerr << "computeW0W1 failed\n";
        return 1;
    }

    // Выводим полученные w0 и w1 в шестнадцатеричном представлении:
    char *w0_hex = BN_bn2hex(w0);
    char *w1_hex = BN_bn2hex(w1);
    std::cout << "w0: " << w0_hex << "\n";
    std::cout << "w1: " << w1_hex << "\n";

    // Освобождаем память
    OPENSSL_free(w0_hex);
    OPENSSL_free(w1_hex);
    BN_free(w0);
    BN_free(w1);

    return 0;
}







//----------------------------------------------------------------------
// Пример использования: генерируем salt, вычисляем w0, w1 из PIN, создаём Spake2p,
// вычисляем X и Y, затем вычисляем секрет и верификаторы (упрощённо).
int main() {
    // Инициализация OpenSSL (не обязательно для современных версий, но на всякий случай)
    OpenSSL_add_all_algorithms();

    uint32_t pin = 123456;
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        std::cerr << "RAND_bytes failed\n";
        return 1;
    }
    int iterations = 10000;

    BIGNUM* w0 = nullptr;
    BIGNUM* w1 = nullptr;
    if (computeW0W1(pin, salt, iterations, &w0, &w1) != 0) {
        std::cerr << "computeW0W1 failed\n";
        return 1;
    }
    // Выводим w0 (опционально):
    char* w0_hex = BN_bn2hex(w0);
    std::cout << "w0: " << w0_hex << "\n";
    OPENSSL_free(w0_hex);

    // Контекст протокола – например, идентификаторы участников.
    std::vector<unsigned char> protocol_context = {'S','P','A','K','E','2','P'};

    // Создаем объект Spake2p (объект принимает владение w0, поэтому не копируем его)
    Spake2p* spake = Spake2p::create(protocol_context, w0);
    // Обратите внимание: после передачи w0 в Spake2p его больше освобождать отдельно.
    if (!spake) {
        BN_free(w1);
        return 1;
    }

    // Вычисляем X и Y
    EC_POINT* X = spake->computeX();
    EC_POINT* Y = spake->computeY();

    // Вычисляем секрет и «верификаторы» с использованием w1.
    std::vector<unsigned char> Ke, hAY, hBX;
    if (!spake->computeSecretAndVerifiersFromY(w1, X, Y, Ke, hAY, hBX)) {
        std::cerr << "computeSecretAndVerifiersFromY failed\n";
    } else {
        std::cout << "Ke: ";
        for (auto b : Ke) printf("%02x", b);
        std::cout << "\nhAY: ";
        for (auto b : hAY) printf("%02x", b);
        std::cout << "\nhBX: ";
        for (auto b : hBX) printf("%02x", b);
        std::cout << "\n";
    }

    // Освобождаем ресурсы
    EC_POINT_free(X);
    EC_POINT_free(Y);
    BN_free(w1);
    delete spake;
    EVP_cleanup();
    return 0;
}



int main() {
    // 1. Выбираем PIN, генерируем соль и задаем число итераций.
    uint32_t pin = 123456;
    std::vector<unsigned char> salt(16);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        std::cerr << "RAND_bytes failed\n";
        return 1;
    }
    int iterations = 10000;

    // 2. Клиент и сервер независимо (но с одинаковыми входными данными) вычисляют w0 и w1.
    BIGNUM* w0_client = nullptr; BIGNUM* w1_client = nullptr;
    if (computeW0W1(pin, salt, iterations, &w0_client, &w1_client) != 0) {
        std::cerr << "Client computeW0W1 failed\n";
        return 1;
    }
    BIGNUM* w0_server = nullptr; BIGNUM* w1_server = nullptr;
    if (computeW0W1(pin, salt, iterations, &w0_server, &w1_server) != 0) {
        std::cerr << "Server computeW0W1 failed\n";
        return 1;
    }

    // 3. На стороне сервера вычисляем L = G · w1 (из w1_server)
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (!group) { std::cerr << "EC_GROUP_new_by_curve_name failed\n"; return 1; }
    EC_POINT* L = computeL(group, w1_server);
    if (!L) { std::cerr << "computeL failed\n"; return 1; }

    // Определим общий контекст протокола.
    std::vector<unsigned char> protocolContext = {'S','P','A','K','E','2','P'};

    // ----- PasePake1 (Клиент)
    // Клиент создаёт объект Spake2p и вычисляет X = G·r + M·w0.
    Spake2p* spakeClient = Spake2p::create(protocolContext, w0_client);
    if (!spakeClient) { std::cerr << "spakeClient creation failed\n"; return 1; }
    EC_POINT* X = spakeClient->computeX();
    // «Передаём» X клиент -> сервер.

    // ----- PasePake2 (Сервер)
    // Сервер создаёт собственный Spake2p (с тем же w0) и вычисляет Y = G·r + N·w0.
    // Затем вычисляет секреты через computeSecretAndVerifiersFromX, используя L.
    Spake2p* spakeServer = Spake2p::create(protocolContext, w0_server);
    if (!spakeServer) { std::cerr << "spakeServer creation failed\n"; return 1; }
    EC_POINT* Y = spakeServer->computeY();
    std::vector<unsigned char> Ke_server, hAY_server, hBX_server;
    if (!spakeServer->computeSecretAndVerifiersFromX(L, X, Y, Ke_server, hAY_server, hBX_server)) {
        std::cerr << "spakeServer computeSecretAndVerifiersFromX failed\n";
        return 1;
    }
    // Сервер «отправляет» клиенту Y и серверный верификатор = hBX_server.

    // ----- PasePake3 (Клиент)
    // Клиент получает Y и серверный верификатор, затем вычисляет секреты через computeSecretAndVerifiersFromY.
    std::vector<unsigned char> Ke_client, hAY_client, hBX_client;
    if (!spakeClient->computeSecretAndVerifiersFromY(w1_client, X, Y, Ke_client, hAY_client, hBX_client)) {
        std::cerr << "spakeClient computeSecretAndVerifiersFromY failed\n";
        return 1;
    }
    // Клиент сравнивает полученный hBX_client с полученным от сервера hBX_server.
    if (hBX_client != hBX_server) {
        std::cerr << "Клиент: неверный серверный верификатор\n";
        return 1;
    }
    // Клиент «отправляет» серверу свой верификатор hAY_client.

    // Сервер проверяет полученный верификатор.
    if (hAY_client != hAY_server) {
        std::cerr << "Сервер: неверный клиентский верификатор\n";
        return 1;
    }

    std::cout << "PASE обмен завершён успешно!\n";

    // Освобождаем ресурсы
    EC_POINT_free(X);
    EC_POINT_free(Y);
    EC_POINT_free(L);
    // Spake2p объекты освобождаются через delete (их деструкторы очищают связанные ресурсы)
    delete spakeClient;
    delete spakeServer;
    BN_free(w1_client);
    BN_free(w0_server);
    BN_free(w1_server);
    // w0_client уже передан в spakeClient (будет освобождён в деструкторе)
    // Освобождаем группу
    // (Group освобождается в деструкторах Spake2p; здесь дополнительное освобождение не требуется.)

    EVP_cleanup();
    return 0;
}
*/
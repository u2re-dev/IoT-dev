const char *client_id = "client@example.com";
const char *server_id = "server@example.com";
const char *password  = "mysecretpassword";

//
int main()
{
    if (sodium_init() < 0) { std::cerr << "Ошибка инициализации libsodium\n"; return 1; }

    //
    unsigned long long passwdlen = std::strlen(password);
    unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
    size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

    //
    unsigned char stored_data[crypto_spake_STOREDBYTES] = {0};
    if (crypto_spake_server_store(stored_data, password, passwdlen, opslimit, memlimit) != 0)
    { std::cerr << "Ошибка создания хранимых данных на сервере.\n"; return 1; }

    // server-side
    unsigned char public_data[crypto_spake_PUBLICDATABYTES] = {0};
    ServerState server_state{};
    if (crypto_spake_step0(server_state, public_data, stored_data) != 0)
    { std::cerr << "Ошибка формирования public_data сервером.\n"; return 1; }

    // client-side
    ClientState client_state{};
    unsigned char response1[crypto_spake_RESPONSE1BYTES] = {0};
    if (crypto_spake_step1(client_state, response1, public_data, password, passwdlen) != 0)
    { std::cerr << "Клиент: ошибка на шаге 1.\n"; return 1; }

    // server-side
    unsigned char response2[crypto_spake_RESPONSE2BYTES] = {0};
    if (crypto_spake_step2(server_state, response2, client_id, std::strlen(client_id), server_id, std::strlen(server_id), stored_data, response1) != 0)
    { std::cerr << "Сервер: ошибка на шаге 2.\n"; return 1; }

    // client-side
    unsigned char response3[crypto_spake_RESPONSE3BYTES] = {0};
    SharedKeys client_shared_keys{};
    if (crypto_spake_step3(client_state, response3, client_shared_keys, client_id, std::strlen(client_id), server_id, std::strlen(server_id), response2) != 0)
    { std::cerr << "Клиент: ошибка на шаге 3.\n"; return 1; }

    // server-side
    SharedKeys server_shared_keys{};
    if (crypto_spake_step4(server_state, server_shared_keys, response3) != 0)
    { std::cerr << "Сервер: ошибка на шаге 4.\n"; return 1; }

    //
    if (sodium_memcmp(client_shared_keys.client_sk.data(), server_shared_keys.client_sk.data(), crypto_spake_SHAREDKEYBYTES) == 0)
    { std::cout << "SPAKE2+ успешно завершён, общий ключ совпадает.\n"; } else
    { std::cerr << "Ошибка: ключи не совпадают!\n"; return 1; }

    //
    return 0;
}

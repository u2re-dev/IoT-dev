
#if CONFIG_ENABLE_CHIP_SHELL
// Буфер для консольных команд
static char console_buffer[101] = {0};

// Обработчик консольной команды для контролируемых (bound) устройств
static esp_err_t app_driver_bound_console_handler(int argc, char **argv)
{
    if (argc == 1 && strncmp(argv[0], "help", sizeof("help")) == 0) {
        printf("Bound commands:\n"
               "\thelp: Print help\n"
               "\tinvoke: <local_endpoint_id> <cluster_id> <command_id> parameters ... \n"
               "\t\tExample: matter esp bound invoke 0x0001 0x0003 0x0000 0x78.\n");
    } else if (argc >= 4 && strncmp(argv[0], "invoke", sizeof("invoke")) == 0) {
        client::request_handle_t req_handle;
        req_handle.type = esp_matter::client::INVOKE_CMD;
        uint16_t local_endpoint_id = strtoul((const char *)&argv[1][2], NULL, 16);
        req_handle.command_path.mClusterId = strtoul((const char *)&argv[2][2], NULL, 16);
        req_handle.command_path.mCommandId = strtoul((const char *)&argv[3][2], NULL, 16);

        if (argc > 4) {
            console_buffer[0] = argc - 4;
            for (int i = 0; i < (argc - 4); i++) {
                if ((argv[4 + i][0] != '0') || (argv[4 + i][1] != 'x') ||
                    (strlen((const char *)&argv[4 + i][2]) > 10)) {
                    ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
                    return ESP_ERR_INVALID_ARG;
                }
                strcpy((console_buffer + 1 + 10 * i), &argv[4 + i][2]);
            }
            req_handle.request_data = console_buffer;
        }
        client::cluster_update(local_endpoint_id, &req_handle);
    } else {
        ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

// Обработчик консольной команды для клиентских (non-bound) устройств
static esp_err_t app_driver_client_console_handler(int argc, char **argv)
{
    if (argc == 1 && strncmp(argv[0], "help", sizeof("help")) == 0) {
        printf("Client commands:\n"
               "\thelp: Print help\n"
               "\tinvoke: <fabric_index> <remote_node_id> <remote_endpoint_id> <cluster_id> <command_id> parameters ... \n"
               "\t\tExample: matter esp client invoke 0x0001 0xBC5C01 0x0001 0x0003 0x0000 0x78.\n"
               "\tinvoke-group: <fabric_index> <group_id> <cluster_id> <command_id> parameters ... \n"
               "\t\tExample: matter esp client invoke-group 0x0001 0x257 0x0003 0x0000 0x78.\n");
    } else if (argc >= 6 && strncmp(argv[0], "invoke", sizeof("invoke")) == 0) {
        client::request_handle_t req_handle;
        req_handle.type = esp_matter::client::INVOKE_CMD;
        uint8_t fabric_index = strtoul((const char *)&argv[1][2], NULL, 16);
        uint64_t node_id = strtoull((const char *)&argv[2][2], NULL, 16);
        req_handle.command_path = {(chip::EndpointId)strtoul((const char *)&argv[3][2], NULL, 16),
                                    0,  // GroupId (не используется для unicast)
                                    strtoul((const char *)&argv[4][2], NULL, 16),
                                    strtoul((const char *)&argv[5][2], NULL, 16),
                                    chip::app::CommandPathFlags::kEndpointIdValid};
        if (argc > 6) {
            console_buffer[0] = argc - 6;
            for (int i = 0; i < (argc - 6); i++) {
                if ((argv[6 + i][0] != '0') || (argv[6 + i][1] != 'x') ||
                    (strlen((const char *)&argv[6 + i][2]) > 10)) {
                    ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
                    return ESP_ERR_INVALID_ARG;
                }
                strcpy((console_buffer + 1 + 10 * i), &argv[6 + i][2]);
            }
            req_handle.request_data = console_buffer;
        }
        auto &server = chip::Server::GetInstance();
        client::connect(server.GetCASESessionManager(), fabric_index, node_id, &req_handle);
    } else if (argc >= 5 && strncmp(argv[0], "invoke-group", sizeof("invoke-group")) == 0) {
        client::request_handle_t req_handle;
        req_handle.type = esp_matter::client::INVOKE_CMD;
        uint8_t fabric_index = strtoul((const char *)&argv[1][2], NULL, 16);
        req_handle.command_path.mGroupId = strtoul((const char *)&argv[2][2], NULL, 16);
        req_handle.command_path.mClusterId = strtoul((const char *)&argv[3][2], NULL, 16);
        req_handle.command_path.mCommandId = strtoul((const char *)&argv[4][2], NULL, 16);
        req_handle.command_path = {0,
                                     (chip::GroupId)strtoul((const char *)&argv[2][2], NULL, 16),
                                     strtoul((const char *)&argv[3][2], NULL, 16),
                                     strtoul((const char *)&argv[4][2], NULL, 16),
                                     chip::app::CommandPathFlags::kGroupIdValid};
        if (argc > 5) {
            console_buffer[0] = argc - 5;
            for (int i = 0; i < (argc - 5); i++) {
                if ((argv[5 + i][0] != '0') || (argv[5 + i][1] != 'x') ||
                    (strlen((const char *)&argv[5 + i][2]) > 10)) {
                    ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
                    return ESP_ERR_INVALID_ARG;
                }
                strcpy((console_buffer + 1 + 10 * i), &argv[5 + i][2]);
            }
            req_handle.request_data = console_buffer;
        }
        client::group_request_send(fabric_index, &req_handle);
    } else {
        ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

// Регистрация консольных команд
static void app_driver_register_commands()
{
    static const esp_matter::console::command_t bound_command = {
        .name = "bound",
        .description = "This can be used to simulate on-device control for bound devices. "
                       "Usage: matter esp bound <bound_command>. Bound commands: help, invoke",
        .handler = app_driver_bound_console_handler,
    };
    esp_matter::console::add_commands(&bound_command, 1);

    static const esp_matter::console::command_t client_command = {
        .name = "client",
        .description = "This can be used to simulate on-device control for client devices. "
                       "Usage: matter esp client <client_command>. Client commands: help, invoke",
        .handler = app_driver_client_console_handler,
    };
    esp_matter::console::add_commands(&client_command, 1);
}
#endif // CONFIG_ENABLE_CHIP_SHELL

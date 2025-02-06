/******************************************************************************************
 * Combined Matter Application
 * 
 * Этот файл объединяет функциональность из app_main.cpp и app_driver.cpp.
 * Здесь собраны общие подключения, определения глобальных переменных, функции Matter приложения,
 * драйверные функции для работы с консолью и клиентскими запросами, а также обработчики кнопок.
 *
 * Лицензия: Public Domain / CC0
 ******************************************************************************************/

// -------------------------------
// Подключения стандартных и системных заголовков
// -------------------------------
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// -------------------------------
// Подключения ESP и Matter
// -------------------------------
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
#include <esp_matter_providers.h>
#include <esp_matter_client.h>

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#include <bsp/esp-bsp.h>
#include <app/server/Server.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVReader.h>

// -------------------------------
// Подключения, зависящие от конфигурации
// -------------------------------
#if CONFIG_SUBSCRIBE_TO_ON_OFF_SERVER_AFTER_BINDING
#include <app/util/binding-table.h>
#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#if CONFIG_DYNAMIC_PASSCODE_COMMISSIONABLE_DATA_PROVIDER
#include <custom_provider/dynamic_commissionable_data_provider.h>
#endif

#if CONFIG_ENABLE_SNTP_TIME_SYNC
#include <app/clusters/time-synchronization-server/DefaultTimeSyncDelegate.h>
#endif

// -------------------------------
// Пространства имён
// -------------------------------
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;  // Для работы с OnOff, Identify и т.п.

// -------------------------------
// Глобальные переменные и константы
// -------------------------------
static const char *TAG = "app_main";
uint16_t switch_endpoint_id = 0;

#if CONFIG_DYNAMIC_PASSCODE_COMMISSIONABLE_DATA_PROVIDER
dynamic_commissionable_data_provider g_dynamic_passcode_provider;
#endif

#if CONFIG_SUBSCRIBE_TO_ON_OFF_SERVER_AFTER_BINDING
static bool do_subscribe = true;
#endif

// -------------------------------
// Функции обратных вызовов Matter
// -------------------------------

// Обработчик событий Matter
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address Changed");
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;
    case chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster:
    {
        ESP_LOGI(TAG, "Binding entry changed");
  #if CONFIG_SUBSCRIBE_TO_ON_OFF_SERVER_AFTER_BINDING
        if(do_subscribe) {
            for (const auto & binding : chip::BindingTable::GetInstance())
            {
                ESP_LOGI(
                    TAG,
                    "Read cached binding type=%d fabrixIndex=%d nodeId=0x" ChipLogFormatX64
                    " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                    binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                    binding.remote, ChipLogValueMEI(binding.clusterId.value_or(0)));
                if (binding.type == MATTER_UNICAST_BINDING && event->BindingsChanged.fabricIndex == binding.fabricIndex)
                {
                    ESP_LOGI(
                        TAG,
                        "Matched accessingFabricIndex with nodeId=0x" ChipLogFormatX64,
                        ChipLogValueX64(binding.nodeId));

                    uint32_t attribute_id = chip::app::Clusters::OnOff::Attributes::OnOff::Id;
                    client::request_handle_t req_handle;
                    req_handle.type = esp_matter::client::SUBSCRIBE_ATTR;
                    req_handle.attribute_path = {binding.remote, binding.clusterId.value(), attribute_id};
                    auto &server = chip::Server::GetInstance();
                    client::connect(server.GetCASESessionManager(), binding.fabricIndex, binding.nodeId, &req_handle);
                    break;
                }
            }
            do_subscribe = false;
        }
  #endif
    }
    break;
    default:
        break;
    }
}

// Обработчик идентификации (например, мигание светодиодом)
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id,
                                         uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

// Обработчик обновления атрибутов
static esp_err_t app_attribute_update_cb(callback_type_t type, uint16_t endpoint_id,
                                           uint32_t cluster_id, uint32_t attribute_id,
                                           esp_matter_attr_val_t *val, void *priv_data)
{
    if (type == PRE_UPDATE) {
        // Здесь можно обработать обновление атрибутов до принятия данных изменений
    }
    return ESP_OK;
}

// -------------------------------
// Matter Client (Драйвер) Callback классы и функции
// -------------------------------

#ifdef CONFIG_SUBSCRIBE_TO_ON_OFF_SERVER_AFTER_BINDING
// Пример реализации колбэка для ReadClient
class MyReadClientCallback : public chip::app::ReadClient::Callback {
public:
    void OnAttributeData(const chip::app::ConcreteDataAttributePath &aPath,
                         chip::TLV::TLVReader *aReader,
                         const chip::app::StatusIB &aStatus) override {
        if (aPath.mClusterId == chip::app::Clusters::OnOff::Id &&
            aPath.mAttributeId == chip::app::Clusters::OnOff::Attributes::OnOff::Id) {
            ESP_LOGI(TAG, "Received OnOff attribute");
        }
    }
    void OnEventData(const chip::app::EventHeader &aEventHeader, chip::TLV::TLVReader *apData,
                     const chip::app::StatusIB *aStatus) override {
        // Обработка event данных
    }
    void OnError(CHIP_ERROR aError) override {
        ESP_LOGI(TAG, "ReadClient Error: %s", ErrorStr(aError));
    }
    void OnDone(chip::app::ReadClient *apReadClient) override {
        ESP_LOGI(TAG, "ReadClient Done");
    }
};
MyReadClientCallback readClientCb;

// Функция подписки для клиента после биндинга
void app_client_subscribe_command_callback(client::peer_device_t *peer_device,
                                             client::request_handle_t *req_handle,
                                             void *priv_data)
{
    uint16_t min_interval = 5;
    uint16_t max_interval = 10;
    bool keep_subscription = true;
    bool auto_resubscribe = true;
    chip::Platform::ScopedMemoryBufferWithSize<chip::app::AttributePathParams> attrb_path;
    attrb_path.Alloc(1);
    client::interaction::subscribe::send_request(peer_device, &req_handle->attribute_path,
                                                 attrb_path.AllocatedSize(), &req_handle->event_path,
                                                 0, min_interval, max_interval, keep_subscription,
                                                 auto_resubscribe, readClientCb);
}
#endif

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

// Обработчики успешной и неуспешной отправки команд
static void send_command_success_callback(void *context, const ConcreteCommandPath &command_path,
                                          const chip::app::StatusIB &status, TLVReader *response_data)
{
    ESP_LOGI(TAG, "Send command success");
}

static void send_command_failure_callback(void *context, CHIP_ERROR error)
{
    ESP_LOGI(TAG, "Send command failure: err :%" CHIP_ERROR_FORMAT, error.Format());
}

// Обработка вызова команды для клиента (unicast)
void app_driver_client_invoke_command_callback(client::peer_device_t *peer_device,
                                               client::request_handle_t *req_handle,
                                               void *priv_data)
{
    if (req_handle->type == esp_matter::client::INVOKE_CMD) {
        char command_data_str[32];
        // Поддержка команд для OnOff и Identify кластеров
        if (req_handle->command_path.mClusterId == OnOff::Id) {
            strcpy(command_data_str, "{}");
        } else if (req_handle->command_path.mClusterId == Identify::Id) {
            if (req_handle->command_path.mCommandId == Identify::Commands::Identify::Id) {
                if (((char *)req_handle->request_data)[0] != 1) {
                    ESP_LOGE(TAG, "Number of parameters error");
                    return;
                }
                snprintf(command_data_str, sizeof(command_data_str), "{\"0:U16\": %ld}",
                         strtoul((const char *)(req_handle->request_data) + 1, NULL, 16));
            } else {
                ESP_LOGE(TAG, "Unsupported command");
                return;
            }
        } else {
            ESP_LOGE(TAG, "Unsupported cluster");
            return;
        }
        client::interaction::invoke::send_request(NULL, peer_device, req_handle->command_path,
                                                  command_data_str, send_command_success_callback,
                                                  send_command_failure_callback, chip::NullOptional);
    }
    return;
}

// Общий callback клиента
void app_driver_client_callback(client::peer_device_t *peer_device, client::request_handle_t *req_handle,
                                 void *priv_data)
{
    if (req_handle->type == esp_matter::client::INVOKE_CMD) {
        app_driver_client_invoke_command_callback(peer_device, req_handle, priv_data);
#ifdef CONFIG_SUBSCRIBE_TO_ON_OFF_SERVER_AFTER_BINDING
    } else if (req_handle->type == esp_matter::client::SUBSCRIBE_ATTR) {
        app_client_subscribe_command_callback(peer_device, req_handle, priv_data);
#endif
    }
    return;
}

// Обработка группового вызова команды
void app_driver_client_group_invoke_command_callback(uint8_t fabric_index,
                                                     client::request_handle_t *req_handle,
                                                     void *priv_data)
{
    if (req_handle->type != esp_matter::client::INVOKE_CMD) {
        return;
    }
    char command_data_str[32];
    if (req_handle->command_path.mClusterId == OnOff::Id) {
        strcpy(command_data_str, "{}");
    } else if (req_handle->command_path.mClusterId == Identify::Id) {
        if (req_handle->command_path.mCommandId == Identify::Commands::Identify::Id) {
            if (((char *)req_handle->request_data)[0] != 1) {
                ESP_LOGE(TAG, "Number of parameters error");
                return;
            }
            snprintf(command_data_str, sizeof(command_data_str), "{\"0:U16\": %ld}",
                     strtoul((const char *)(req_handle->request_data) + 1, NULL, 16));
        } else {
            ESP_LOGE(TAG, "Unsupported command");
            return;
        }
    } else {
        ESP_LOGE(TAG, "Unsupported cluster");
        return;
    }
    client::interaction::invoke::send_group_request(fabric_index, req_handle->command_path, command_data_str);
}

// -------------------------------
// Обработчик нажатия кнопки. Переключает состояние OnOff (Toggle)
// -------------------------------
static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    client::request_handle_t req_handle;
    req_handle.type = esp_matter::client::INVOKE_CMD;
    req_handle.command_path.mClusterId = OnOff::Id;
    req_handle.command_path.mCommandId = OnOff::Commands::Toggle::Id;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, &req_handle);
    lock::chip_stack_unlock();
}

// -------------------------------
// Инициализация драйвера: настройка кнопки и регистрация консольных команд
// -------------------------------
app_driver_handle_t app_driver_switch_init()
{
    button_handle_t btns[BSP_BUTTON_NUM];
    ESP_ERROR_CHECK(bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM));
    ESP_ERROR_CHECK(iot_button_register_cb(btns[0], BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL));

  #if CONFIG_ENABLE_CHIP_SHELL
    app_driver_register_commands();
  #endif
    client::set_request_callback(app_driver_client_callback,
                                 app_driver_client_group_invoke_command_callback, NULL);

    return (app_driver_handle_t)btns[0];
}

// -------------------------------
// Точка входа приложения (app_main)
// -------------------------------
extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    // Инициализация NVS
    nvs_flash_init();

    // Инициализация драйвера (например, переключателя [button] и его регистрация для сброса)
    app_driver_handle_t switch_handle = app_driver_switch_init();
    app_reset_button_register(switch_handle);

    // Создание Matter узла, добавление обязательного Root Node с endpoint 0
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

  #if CONFIG_ENABLE_SNTP_TIME_SYNC
    endpoint_t *root_node_ep = endpoint::get_first(node);
    ABORT_APP_ON_FAILURE(root_node_ep != nullptr, ESP_LOGE(TAG, "Failed to find root node endpoint"));

    cluster::time_synchronization::config_t time_sync_cfg;
    static chip::app::Clusters::TimeSynchronization::DefaultTimeSyncDelegate time_sync_delegate;
    time_sync_cfg.delegate = &time_sync_delegate;
    cluster_t *time_sync_cluster = cluster::time_synchronization::create(root_node_ep, &time_sync_cfg, CLUSTER_FLAG_SERVER);
    ABORT_APP_ON_FAILURE(time_sync_cluster != nullptr, ESP_LOGE(TAG, "Failed to create time_sync_cluster"));

    cluster::time_synchronization::feature::time_zone::config_t tz_cfg;
    cluster::time_synchronization::feature::time_zone::add(time_sync_cluster, &tz_cfg);
  #endif

    // Добавление on_off переключателя
    on_off_switch::config_t switch_config;
    endpoint_t *endpoint = on_off_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, switch_handle);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create on off switch endpoint"));

    // Добавление группы (groups cluster) к endpoint переключателя
    cluster::groups::config_t groups_config;
    cluster::groups::create(endpoint, &groups_config, CLUSTER_FLAG_SERVER | CLUSTER_FLAG_CLIENT);

    switch_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Switch created with endpoint_id %d", switch_endpoint_id);

  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    // Конфигурация OpenThread
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config  = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
  #endif

  #if CONFIG_DYNAMIC_PASSCODE_COMMISSIONABLE_DATA_PROVIDER
    // Установка кастомного провайдера данных для commissionable (вызывается перед esp_matter::start())
    esp_matter::set_custom_commissionable_data_provider(&g_dynamic_passcode_provider);
  #endif

    // Запуск стека Matter
    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

  #if CONFIG_ENABLE_CHIP_SHELL
    // Регистрация диагностических, Wi-Fi и других команд в консоли
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::init();
  #endif
}
 

/* 
   Примечание:
   - Этот файл объединяет функции, относящиеся к Matter уровню (создание узла, endpoint'ов, кластеров)
     и функцию драйвера, в том числе обработчики консольных команд, запросов клиента и событий кнопки.
   - Условная компиляция (#if CONFIG_...) позволяет включать/отключать функционал на этапе сборки.
   - Для тестирования и дальнейшей отладки рекомендуется собрать проект с активированными
     соответствующими конфигурациями.
*/


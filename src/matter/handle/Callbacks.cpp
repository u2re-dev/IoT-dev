#include "Callbacks.hpp"

//
#include "../client/ClientRemote.hpp"
#include "../client/ClientRequest.hpp"

//
#include <esp_log.h>
#include <chip/CHIPLogging.h>
#include <inttypes.h>

//
static const char *TAG = "Callbacks";

//
namespace Callbacks {
    void cmdSuccess(void *context, const chip::app::ConcreteCommandPath &command_path, const chip::app::StatusIB &status, chip::TLV::TLVReader *response_data) { ESP_LOGI(TAG, "Command sent successfully"); }
    void cmdFailed(void *context, CHIP_ERROR error) { ESP_LOGI(TAG, "Command failure: err: %" CHIP_ERROR_FORMAT, error.Format()); }
    void appEvent(const ChipDeviceEvent *event, intptr_t arg)
    {
        switch (event->Type) {
            case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged: ESP_LOGI(TAG, "Interface IP Address Changed"); break;
            case chip::DeviceLayer::DeviceEventType::kCommissioningComplete: ESP_LOGI(TAG, "Commissioning complete"); break;
            case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired: ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired"); break;
            case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted: ESP_LOGI(TAG, "Commissioning session started"); break;
            case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped: ESP_LOGI(TAG, "Commissioning session stopped"); break;
            case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened: ESP_LOGI(TAG, "Commissioning window opened"); break;
            case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed: ESP_LOGI(TAG, "Commissioning window closed"); break;
            case chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster: ESP_LOGI(TAG, "Binding entry changed"); break;
            default: break;
        }
    }

    esp_err_t identification(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
    {
        ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
        return ESP_OK;
    }

    esp_err_t attributeUpdate(callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
    {
        if (type == PRE_UPDATE) {
            ESP_LOGI(TAG, "Pre-update attribute callback (cluster: %u attribute: %u)", cluster_id, attribute_id);
        }
        return ESP_OK;
    }

    void clientRequestAndConnect(client::peer_device_t *peer_device, client::request_handle_t *req_handle, void *priv_data)
    {
        ESP_LOGI(TAG, "Client request callback - connect to remote device");
        auto clientRequest = ClientRequest(peer_device, req_handle);
        ((NodeRemote*)priv_data)->handleClientRequest(clientRequest);
    }

    void groupRequestAndConnect(client::peer_device_t *peer_device, client::request_handle_t *req_handle, void *priv_data)
    {
        ESP_LOGI(TAG, "Group request callback - connect for group request");
        // Аналогичная обработка для групповых запросов.
    }

} // namespace Callbacks

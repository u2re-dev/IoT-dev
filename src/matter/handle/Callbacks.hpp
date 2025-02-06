#ifndef CALLBACKS_H
#define CALLBACKS_H

//
#include <esp_err.h>
#include <esp_matter.h>
#include <chip/CHIPError.h>
#include <client/CHIPClient.h>

// Все callback‑функции объединяем в один неймспейс.
namespace Callbacks {

    // Системное событие (например, изменение IP, commissioning и т.д.)
    void appEvent(const ChipDeviceEvent *event, intptr_t arg);
    void cmdSuccess(void *context, const chip::app::ConcreteCommandPath &command_path, const chip::app::StatusIB &status, chip::TLV::TLVReader *response_data);
    void cmdFailed(void *context, CHIP_ERROR error);

    // Идентификация (напр., при вызове Identify Cluster)
    esp_err_t identification(identification::callback_type_t type, uint16_t endpoint_id,  uint8_t effect_id, uint8_t effect_variant, void *priv_data);
    esp_err_t attributeUpdate(callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);

    // Callback для установления соединения с устройством (для unicast)
    void clientRequestAndConnect(client::peer_device_t *peer_device, client::request_handle_t *req_handle, void *priv_data);
    void groupRequestAndConnect(client::peer_device_t *peer_device, client::request_handle_t *req_handle, void *priv_data);
} // namespace Callbacks

#endif // CALLBACKS_H

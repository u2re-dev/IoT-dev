#pragma once

//
#ifdef USE_ESP_MATTER
#include <esp_err.h>
#include <esp_matter.h>
#include <chip/CHIPError.h>
#include <client/CHIPClient.h>

//
namespace Callbacks {
    void appEvent(const ChipDeviceEvent *event, intptr_t arg);
    void cmdSuccess(void *context, const chip::app::ConcreteCommandPath &command_path, const chip::app::StatusIB &status, chip::TLV::TLVReader *response_data);
    void cmdFailed(void *context, CHIP_ERROR error);

    //
    esp_err_t identification(identification::callback_type_t type, uint16_t endpoint_id,  uint8_t effect_id, uint8_t effect_variant, void *priv_data);
    esp_err_t attributeUpdate(callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);

    //
    void clientRequestAndConnect(client::peer_device_t *peer_device, client::request_handle_t *req_handle, void *priv_data);
    void  groupRequestAndConnect(client::peer_device_t *peer_device, client::request_handle_t *req_handle, void *priv_data);
} // namespace Callbacks
#endif

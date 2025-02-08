#include "NodeSelf.hpp"

//
#include "../io/Callbacks.hpp"

//
#ifdef ENABLE_MATTER
#include <esp_log.h>
#include <app_priv.h>  // для app_attribute_update_cb, app_identification_cb

static const char *TAG = "MatterNodeSelf";

MatterNodeSelf::~MatterNodeSelf() { }
MatterNodeSelf::MatterNodeSelf()
    : mNode(nullptr), mSwitchEndpointId(0) {}

esp_err_t MatterNodeSelf::init()
{
    node::config_t node_config = {};
    mNode = node::create(&node_config, Callbacks::attributeUpdate, Callbacks::identification);
    if (mNode == nullptr) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t MatterNodeSelf::start()
{
    esp_err_t err = esp_matter::start(Callbacks::appEvent);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Matter, error: %d", err);
    }
    return err;
}
#endif

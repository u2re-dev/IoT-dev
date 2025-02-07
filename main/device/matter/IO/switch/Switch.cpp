#include "Switch.hpp"

//
#ifdef ENABLE_MATTER
#include <esp_log.h>
#include <esp_matter.h>

//
static const char *TAG = "Switch";

//
Switch::Switch() { }
Switch::~Switch() { }

//
esp_err_t createOnOffSwitchEndpoint(void *switchHandle, mEndpoint* const& mEndpointId)
{
    on_off_switch::config_t switch_config = {};
    endpoint_t *endpoint = on_off_switch::create(mNode, &switch_config, ENDPOINT_FLAG_NONE, switchHandle);
    if (endpoint == nullptr) {
        ESP_LOGE(TAG, "Failed to create on_off switch endpoint");
        return ESP_FAIL;
    }
    cluster::groups::config_t groups_config = {};
    cluster::groups::create(endpoint, &groups_config, CLUSTER_FLAG_SERVER | CLUSTER_FLAG_CLIENT);
    mEndpointId = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "On-off switch endpoint created with id %d", mEndpointId);
    return ESP_OK;
}
#endif

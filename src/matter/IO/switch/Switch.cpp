#include "HostIO.hpp"

//
#include <esp_log.h>
#include <app_driver.h>
#include <app_reset.h>

static const char *TAG = "HostIO";

HostIO::HostIO()
    : mSwitchHandle(nullptr)
{
}

HostIO::~HostIO()
{
    // Очистка ресурсов, если необходимо
}

esp_err_t HostIO::init()
{
    return initSwitch();
}

esp_err_t HostIO::initSwitch()
{
    mSwitchHandle = app_driver_switch_init();
    if (mSwitchHandle == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize switch");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t HostIO::registerIOCallbacks()
{
    app_reset_button_register(mSwitchHandle);
    return ESP_OK;
}

app_driver_handle_t HostIO::getSwitchHandle() const
{
    return mSwitchHandle;
}

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

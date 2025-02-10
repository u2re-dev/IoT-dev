#include "LocalEndpoint.hpp"

//
#ifdef USE_ESP_MATTER
#include <esp_log.h>

//
#include <client/CHIPClient.h>
#include <lock/CHIPLock.h>

//
static const char *TAG = "Endpoint";

//
LocalEndpoint::LocalEndpoint(node_t *node) : mPeerDevice(nullptr), mEndpoint(nullptr) {}
LocalEndpoint::~LocalEndpoint() { }

//
esp_err_t LocalEndpoint::updateCluster(const client::request_handle_t &reqHandle)
{
    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(mEndpoint, &reqHandle);
    lock::chip_stack_unlock();
    return ESP_OK;
}
#endif
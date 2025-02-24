#include "NodeSelf.hpp"

#ifdef USE_ESP_MATTER
//
void MatterNodeSelf::syncWithNTP()
{
#ifdef CONFIG_ENABLE_SNTP_TIME_SYNC
    endpoint_t *root_node_ep = endpoint::get_first(mNode);
    if (root_node_ep == nullptr) {
        ESP_LOGE(TAG, "Failed to get root endpoint for NTP sync");
        return;
    }
    cluster::time_synchronization::config_t time_sync_cfg = {};
    static chip::app::Clusters::TimeSynchronization::DefaultTimeSyncDelegate time_sync_delegate;
    time_sync_cfg.delegate = &time_sync_delegate;
    cluster_t *time_sync_cluster = cluster::time_synchronization::create(root_node_ep, &time_sync_cfg, CLUSTER_FLAG_SERVER);
    if (time_sync_cluster == nullptr) {
        ESP_LOGE(TAG, "Failed to create time sync cluster");
        return;
    }
    cluster::time_synchronization::feature::time_zone::config_t tz_cfg = {};
    cluster::time_synchronization::feature::time_zone::add(time_sync_cluster, &tz_cfg);
#endif
}
#endif

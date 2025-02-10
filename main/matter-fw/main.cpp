#ifdef USE_ESP_MATTER
#include "stack/NodeSelf.hpp"
#include "stack/HostIO.hpp"
#include "stack/NodeRemote.hpp"

//
#include <nvs_flash.h>
#include <esp_log.h>

//
static const char *TAG = "Main";

//
extern "C" void alt_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NVS: %d", err);
        return;
    }

    // Создаём локальный Matter узел
    MatterNodeSelf nodeSelf;
    err = nodeSelf.init();

    //
    //nodeSelf.syncWithNTP();
    err = nodeSelf.startMatter();

    //
    auto& client = RemoteClient(nodeSelf, 0xBC5C01, 1);

    //
    node.subscribe(endPoint, OnOff::Id);

    //node.request(1, 0xBC5C01, req_handle);
}
#endif

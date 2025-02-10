#pragma once

//
#ifdef USE_ESP_MATTER
#include <esp_err.h>
#include <esp_matter.h>

// Класс MatterNodeSelf отвечает за локальную инициализацию узла Matter, создание эндпоинтов и кластеров.
class MatterNodeSelf {
public:
    MatterNodeSelf();
    ~MatterNodeSelf();

    //
    esp_err_t init();
    esp_err_t start();

    //
    node_t *getNode() const { return mNode; }

protected:
    void syncWithNTP();

private:
    node_t *mNode;
    uint16_t mSelfEndpoint;

    //
    ClientRemote mRemote;
};
#endif

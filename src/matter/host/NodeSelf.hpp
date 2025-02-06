#ifndef NODE_SELF_H
#define NODE_SELF_H

//
#include <esp_err.h>
#include <esp_matter.h>

// Класс MatterNodeSelf отвечает за локальную инициализацию узла Matter, создание эндпоинтов и кластеров.
class MatterNodeSelf {
public:
    MatterNodeSelf();
    ~MatterNodeSelf();

    // Инициализация узла (создание Node, а затем эндпоинтов/кластеров)
    esp_err_t init();
    esp_err_t startMatter();

    // Пример синхронизации по NTP (например, через TimeSynchronization Cluster)
    void syncWithNTP();

    // Возвращает указатель на локальный node_t
    node_t *getNode() const { return mNode; }

private:
    node_t *mNode;
    uint16_t mSelfEndpoint;

    //
    ClientRemote mRemote;
};

#endif // NODE_SELF_H

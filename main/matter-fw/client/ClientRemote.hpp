#pragma once

//
#include "../handle/Callbacks.hpp"

//
#ifdef USE_ESP_MATTER
#include <esp_err.h>
#include <esp_matter.h>
#include <client/CHIPClient.h>

// Класс Remote Node – отвечает за связь с удалённым узлом (установка сессии, отправка команд…
class ClientRemote {
protected:
    void connectToDevice(uintptr_t fabricId, uintptr_t nodeId);
    void handleClientRequest(ClientRequest const& request);

public:
    //std::vector<ClientRequest> requests = {};
    node_t *node = nullptr;
    uintptr_t fabricId, nodeId;

    //
    explicit ClientRemote(node_t *node, uintptr_t nodeId, uintptr_t fabricId);
    ~ClientRemote();

    // subscribe remote endpoint to changes in local cluster
    void subscribe(uintptr_t clusterId, LocalEndpoint *local, uintptr_t remoteId);
    //void invoke(uintptr_t clusterId);
};

#endif

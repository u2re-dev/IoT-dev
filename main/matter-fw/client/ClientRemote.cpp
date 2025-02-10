#include "ClientRemote.hpp"
#ifdef USE_ESP_MATTER

//
#include <esp_log.h>

//
#include <chip/CHIPServer.h>

//
static const char *TAG = "ClientRemote";

// node-based client remote interaction
ClientRemote::ClientRemote(node_t *node, uintptr_t nodeId, uintptr_t fabricId) : fabricId(fabricId), nodeId(nodeId), node(node) {
    client::set_request_callback(Callbacks::clientRequestAndConnect, Callbacks::groupRequestAndConnect, this);
}

//
ClientRemote::~ClientRemote()
{
    // Освобождение ресурсов, если необходимо.
}



// make request to client device
void ClientRemote::initClientRequest(client::request_handle_t const& mRequesHandle) {
    auto &server = chip::Server::GetInstance();
    client::connect(server.GetCASESessionManager(), fabricIdx, nodeId, &mRequesHandle);
    ESP_LOGI(TAG, "Initiated connection to device, fabricIdx: %d, nodeId: %llu", (int)fabricId, (unsigned long long)nodeId);
}

// decide what to do with ClientRequest
void ClientRemote::handleClientRequest(ClientRequest const& request) {
    
}

#endif

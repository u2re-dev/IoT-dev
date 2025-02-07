#pragma once

//
#ifdef ENABLE_MATTER
#include <esp_err.h>
#include <esp_matter.h>
#include <client/CHIPClient.h>

//
#include "../io/Callbacks.hpp"

// Класс Remote Node – отвечает за связь с удалённым узлом (установка сессии, отправка команд…
class ClientRequest {
public:
    explicit ClientRequest(client::peer_device_t *peerDev, client::request_handle_t *mReqH);
    ~ClientRequest();

    //
    void invokeCommand(const char *commandDataStr);
    void subscribeAttribute();

    //
private: //
    client::peer_device_t *mPeerDevice;
    client::request_handle_t *mReqHandle;
};
#endif

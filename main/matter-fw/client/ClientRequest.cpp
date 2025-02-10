#include "ClientRequest.hpp"

//
#ifdef USE_ESP_MATTER
#include <esp_log.h>

//
#include <chip/CHIPServer.h>

//
static const char *TAG = "ClientRequest";

//
ClientRequest::ClientRequest(client::peer_device_t *peerDev, client::request_handle_t *mReqH) : mPeerDevice(peerDev), mRequesHandle(mReqH) {}
ClientRequest::~ClientRequest() {}

// when handling as invoke
void ClientRequest::invokeCommand(const char *commandDataStr)
{   //
    client::interaction::invoke::send_request(nullptr, mPeerDevice, 
        mReqHandle.command_path, commandDataStr, 
        Callbacks::cmdSuccess, 
        Callbacks::cmdFailed, 
        chip::NullOptional);
}

// when handling as subscribe
void ClientRequest::subscribeAttribute()
{
    client::interaction::subscribe::send_request(
        mPeerDevice, &mReqHandle.attribute_path, 0,
        &mReqHandle.event_path, 0, 
        /* min_interval */ 5, 
        /* max_interval */ 10,
        /* keep_subscription */ true, 
        /* auto_resubscribe */ true,
        Callbacks::readClient);
}
#endif

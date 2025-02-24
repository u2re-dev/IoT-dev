#pragma once

//
#ifdef USE_ESP_MATTER
#include <esp_err.h>
#include <esp_matter.h>

//
class LocalEndpoint {
public:
    explicit LocalEndpoint(node_t *node);
    ~LocalEndpoint();
    
protected:
    node_t *mNode;
    endpoint_t *mEndpoint;
};
#endif

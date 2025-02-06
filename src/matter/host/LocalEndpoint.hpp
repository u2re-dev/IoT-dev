#ifndef NODE_REMOTE_H
#define NODE_REMOTE_H

//
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

#endif // NODE_REMOTE_H

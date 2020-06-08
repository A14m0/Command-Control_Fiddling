#pragma once


#include <dlfcn.h>
#include "transport.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "connection.h"

class Server : public Common 
{
private:
    std::vector<pthread_t> *sessions;
    std::vector<ptransport_t *> *api_handles;
    std::queue<class ConnectionInstance *> *shell_queue;
    
    class Log *logger;
    ptransport_t transport;
    int master_socket;
public:
    Server();
    ~Server();

    int listen_instance(ptransport_t transport, int port);
    int listen_instance(int transport_id, int port);
    std::queue<class ConnectionInstance *> *get_shell_queue();
    std::vector<ptransport_t *> *get_api_handles();

    void add_transport_api(ptransport_t* transport);
};

void *init_instance(void *args);
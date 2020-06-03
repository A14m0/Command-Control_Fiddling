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
    std::queue<class ConnectionInstance *> *shell_queue;
    
    class Log *logger;
    ptransport_t transport;
    int master_socket;
public:
    Server();
    ~Server();

    int listen_instance(ptransport_t transport);
    std::queue<class ConnectionInstance *> *get_shell_queue();
};

void *init_instance(void *args);
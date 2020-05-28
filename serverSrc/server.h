#pragma once


#include <dlfcn.h>
#include "transport.h"
#include "log.h"
//#include "list.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "connection.h"

class Server {
private:
    std::vector<class ConnectionInstance *> *sessions;
    std::queue<class ConnectionInstance *> *shell_queue;
    
    class Log *logger;
    ptransport_t transport;
    int master_socket;
public:
    Server();
    ~Server();

    void add_instance(class ConnectionInstance *instance);
    int bind_instance(int index);
    int listen_instance(int index);
    int listen_instance(class ConnectionInstance *instance);
    std::queue<class ConnectionInstance *> *get_shell_queue();
    class Log * get_log();
};
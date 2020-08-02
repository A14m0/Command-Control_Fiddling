#pragma once


#include <dlfcn.h>
#include "transport.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "connection.h"
#include "server_module.h"

class Server : public Common 
{
private:
    std::vector<class ServerModule *> *modules;
    std::queue<class ConnectionInstance *> *shell_queue;
    
    class Log *logger;
    ptransport_t transport;
    //int master_socket;
public:
    Server();
    ~Server();

    int listen_instance(class ServerModule *module, int port);
    std::queue<class ConnectionInstance *> *get_shell_queue();
    std::vector<class ServerModule *> *get_modules();

    void add_module(void *handle);
    static void *handle_instance(class Server *server, void *handle, bool reload);
    void reload_backends();

    class ServerModule *get_module_from_id(int id);
};

void *init_instance(void *args);
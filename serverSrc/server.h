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
    std::vector<void (*)()> *module_handles;
    std::vector<void *> *shared_lib_handles;
    std::vector<int> *handle_ids;
    std::vector<const char *> *handle_names;
    std::queue<class ConnectionInstance *> *shell_queue;
    
    class Log *logger;
    ptransport_t transport;
    //int master_socket;
public:
    Server();
    ~Server();

    int listen_instance(ptransport_t transport, int port);
    int listen_instance(int transport_id, int port);
    std::queue<class ConnectionInstance *> *get_shell_queue();
    std::vector<ptransport_t *> *get_api_handles();
    std::vector<int> *get_handle_ids();
    std::vector<const char *> *get_handle_names();

    void add_transport_api(ptransport_t* transport, const char *name, int id);
    void add_module(void (*entrypoint)(), const char *name, int id);
    void add_handle(void *handle);
    static void *handle_instance(class Server *server, void *handle, bool reload);
    void reload_backends();

    int get_id_from_handle(void *handle);
    char *get_name_from_handle(void *handle);
};

void *init_instance(void *args);
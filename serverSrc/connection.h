#pragma once

#include "transport.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "server.h"
#include "b64.h"
#include "shell.h"


class ConnectionInstance : public Common
{
private:
    class Server *server;
    ptransport_t transport;
    int shell_finished = 0;
    void* data;
    pthread_t thread;
    int type = 0;
    char *agent_name;
public:
    ConnectionInstance();
    ~ConnectionInstance();

    // set functions
    void set_transport(ptransport_t transport);
    void set_thread(pthread_t thread);
    void set_data(void* data);
    void set_server(class Server* server);
    void shell_finish();

    // get functions
    ptransport_t get_transport();
    void *get_data();
    int get_info(char *ptr);
    int get_ports(char *ptr);
    
    // handler functions
    int handle_connection();
    int authenticate(void *sess);
    int manager_handler();
    int agent_handler();
    
    // misc functions
    int reverse_shell();
    int manager_shell_session(ShellComms *session);
    int agent_shell_session(ShellComms *session);
    int send_transports();
    int setup_transport(char *ptr);
    int send_info(char *ptr);
    int upload_file(char *ptr);
    int send_loot(char *ptr);
    int download_file(char *ptr, int is_manager, char *extra);
};


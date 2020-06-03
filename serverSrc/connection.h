#pragma once

#include "transport.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "server.h"
#include "b64.h"


class ConnectionInstance : public Common
{
private:
    ptransport_t transport;
    int shell_finished = 0;
    pClientDat data;
    pthread_t thread;
public:
    ConnectionInstance();
    ~ConnectionInstance();

    // set functions
    void set_transport(ptransport_t transport);
    void set_thread(pthread_t thread);
    void shell_finish();

    // get functions
    ptransport_t get_transport();
    pClientDat get_data();

    // fetch functions
    void get_info(char *ptr);
    void get_ports(char *ptr);
    
    // handler functions
    int handle_connection();
    void authenticate(void *sess);
    void manager_handler();
    void agent_handler();
    
    // misc functions
    void reverse_shell();
    void send_transports();
    void setup_transport(char *ptr);
    int send_info(char *ptr);
    int download_file(char *ptr, int is_manager, char *extra);
};


#pragma once

#include "transport.h"
#include "ssh_transport.h"
#include "log.h"
//#include "list.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "server.h"


class ConnectionInstance
{
private:
    class Log *logger;
    class ServerTransport *transport;
    class Server *server;
    int shell_finished = 0;
    pClientDat data;
    pthread_t thread;
public:
    ConnectionInstance(class Server *server);
    ~ConnectionInstance();

    // set functions
    void set_transport(class ServerTransport *transport);
    void set_thread(pthread_t thread);
    void shell_finish();

    // get functions
    class Log *get_logger();
    class ServerTransport *get_transport();
    class Server *get_server();
    pClientDat get_data();

    // fetch functions
    void get_info(char *ptr);
    void get_ports(char *ptr);
    
    // handler functions
    static void *handle_connection(void *input);
    void authenticate(void *sess);
    void manager_handler();
    void agent_handler();
    
    // misc functions
    void reverse_shell();
    void setup_transport(char *ptr);
};


#pragma once

#include "transport.h"
#include "ssh_transport.h"
#include "log.h"
#include "list.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"


class ConnectionInstance
{
private:
    class Log *logger;
    class List *list;
    class ServerTransport *transport;
public:
    ConnectionInstance();
    ~ConnectionInstance();

    // set functions
    void set_transport(class ServerTransport *transport);

    // get functions
    class Log *get_logger();
    class List *get_list();

    // fetch functions
    void get_info(char *ptr);
    void get_ports(char *ptr);
    
    // handler functions
    static void *handle_connection(void *input);
    void authenticate(void *sess);
    void manager_handler();
    void agent_handler();
    
    // file functions
    void reverse_shell();
};


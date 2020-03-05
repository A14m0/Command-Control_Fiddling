#pragma once

#include "transport.h"
#include "ssh_transport.h"
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
    class ServerTransport *transport;
    int master_socket;
public:
    Server();
    ~Server();

    void add_instance(class ConnectionInstance *instance);
    int bind_instance(int index);
    int listen_instance(int index);
    std::queue<class ConnectionInstance *> *get_shell_queue();
    class Log * get_log();
};
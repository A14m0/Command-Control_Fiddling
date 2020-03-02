#pragma once

#include <vector>
#include "transport.h"
#include "ssh_transport.h"
#include "log.h"
#include "list.h"
#include "authenticate.h"
#include "agents.h"
#include "common.h"
#include "connection.h"

class Server {
private:
    std::vector<ConnectionInstance *> *sessions;
    class Log *logger;
    class List *list;
    class ServerTransport *transport;
    int master_socket;
public:
    Server();
    ~Server();

    int bind_port(int port);
    int transport_listen(int index);
};
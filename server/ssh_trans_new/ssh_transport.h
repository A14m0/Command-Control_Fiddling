#pragma once

#include "api.h"
#include "common.h"
#include "netinst.h"

#include <stdlib.h>
#include <libssh/libssh.h>
#include <libssh/server.h>

#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#define KEYS_FOLDER "/etc/ssh/"
#endif
#endif



#define REQ_NONE 0
#define REQ_EXEC 1
#define REQ_TASKING 2
#define REQ_TTY 3

void *generate_transport(NetInst *parent);


class SshTransport : TransportAPI{
private:
    int portno = 22;
    char *agent_name = "NONE";
    ssh_bind sshbind;
    ssh_session session;
    ssh_channel channel;
    NetInst *p_ref;

    int Authenticate();
    int DetermineHandler();
    int write(char *buffer, int len);
    //int read(char **buffer, int len);
    char *read(int len);

public:
    SshTransport(NetInst *parent_ref);
    ~SshTransport();


    // gets the next available tasking
    // returns tasking struct with OP_NODATA if nothing available
    api_return fetch_tasking() override;
    // pushes a task update to connected client
    api_return push_tasking(ptask_t task) override;
    // starts listening for this instance
    api_return listen() override;
    // returns the currently connected agent's name
    api_return get_aname() override;
    // sets the listening port of the instance
    api_return set_port(int portno) override;
    // returns transport id
    int get_id() override;
    // returns transport name
    const char* get_tname() override;
};


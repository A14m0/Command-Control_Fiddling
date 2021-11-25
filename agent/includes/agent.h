#pragma once

#include <errno.h>
#include <libssh/libssh.h>
#include "common.h"
#include "shell.h"
#include "tasking.h"
#include "beacon.h"


// define our agent class
class Agent
{
private:
    ssh_session session;
    ssh_channel chan;
    int connect_ssh();
    int authenticate();
    int init_channel();
    int write(char *buffer, int len);
    int read(char **buffer, int len);
    unsigned long read_task();
    AgentJob *parse_tasking(unsigned long tasking);
public:
    Agent(/* args */);
    ~Agent();
    int direct_forwarding();
    int upload_file(void *data);    // REWORK
    int download_file(void *data);  // REWORK
    int run();
};








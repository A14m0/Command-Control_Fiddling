#pragma once
#include "common.h"
#include "config.h"
#include "execs.h"
#include "list.h"
#include "log.h"

class AgentInformationHandler
{
private:
    class List nodeHandler;
    class Log logger;
public:
    AgentInformationHandler();
    ~AgentInformationHandler();
    int init(char *agent_id);
    int register_agent(char *username, char *password);
    int compile(char *ip, char *port);
    int task(int operation, char *agent, char *opt);
    int write_beacon(char *id, char *beacon);
    int write_info(char *id, char *connection_time, char *hostname, char *ip_addr, char *interfaces, char *proc_owner);
    int write_format(char *path);
    char *get_tasking(char *agent_id);
    struct ret *gen_creds();
};

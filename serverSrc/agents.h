#pragma once
#include "common.h"
#include "config.h"
#include "execs.h"
#include "list.h"
#include "authenticate.h"

typedef struct _agent_tasking{
    int operation;
    char *opts;
} AgentTasking, *pAgentTasking;


class AgentInformationHandler
{
private:
    class List *nodeHandler;
    class Log *logger;
public:
    AgentInformationHandler();
    ~AgentInformationHandler();
    static int init(char *agent_id);
    static int register_agent(char *username, char *password);
    static int compile(char *ip, char *port);
    static int task(int operation, char *agent, char *opt);
    static int write_beacon(char *id, char *beacon);
    static int write_info(char *id, char *connection_time, char *hostname, char *ip_addr, char *interfaces, char *proc_owner);
    static int write_format(char *path);
    static char *get_tasking(char *agent_id);
    static pPasswd gen_creds();
};

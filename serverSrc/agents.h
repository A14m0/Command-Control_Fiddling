#pragma once
#include "common.h"
#include "execs.h"
#include "typedefs.h"
#include "authenticate.h"

typedef struct _agent_tasking{
    int operation;
    char *opts;
} AgentTasking, *pAgentTasking;


class AgentInformationHandler
{
public:
    //AgentInformationHandler();
    //~AgentInformationHandler();
    static int init(const char *agent_id);

    static int register_agent(const char *username, 
                            const char *password);

    static int register_agent(char *line);

    static int task(const int operation, const char *agent, 
                    const char *opt);

    static int write_beacon(const char *id, const char *beacon);

    static int write_default_manifest(char *path);

    static char *get_tasking(const char *agent_id);
    
    static pPasswd gen_creds();
};

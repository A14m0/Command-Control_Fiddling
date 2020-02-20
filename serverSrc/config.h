#pragma once

#define DATA_FILE "agents/agents.dat"
#define COMPILE "gcc -lssh -lcurl -o out/client.out out/client.c out/agent.c out/b64.c out/beacon.c"
#define HAVE_ARGP_H
#define MAX_CONN 10
#define AGENT_SOURCE "../clientSrc/"
#pragma once
#include "config.h"
#include "execs.h"

#include <libssh/libssh.h>
#include <libssh/server.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#define COMPILE "gcc -lssh -lcurl -o out/client.out out/client.c out/agent.c out/b64.c"

void agent_init(char *agent_id);
int agent_get_tasking(char *agent_id, char *tasking);
void agent_compile(char *ip, char *port);
void agent_register(char *id, char *port);
void agent_task(int operation, char *agent, char *opt);
struct ret *agent_gen_creds();

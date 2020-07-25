#pragma once

#include "../transport.h"
#include "../typedefs.h"
#include "../b64.h"
#include "../common.h"
#include "../execs.h"
#include "../misc.h"
#include "../authenticate.h"
#include "../agents.h"

#include <libssh/libssh.h>
#include <libssh/server.h>

#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#define KEYS_FOLDER "/etc/ssh/"
#endif
#endif


    
int authenticate(void** instance_struct);
int init(void* dat);
int end(void* instance_struct);

// generic protocol handlers
int send_ok(void* instance_struct);
int send_err(void* instance_struct);
int listen(void* instance_struct);
int read(void* instance_struct, char **buff, int length);
int write(void* instance_struct, const char *buff, int length);
    
int determine_handler(void* instance_struct);
int upload_file(void* instance_struct, const char *ptr, int is_module);
int download_file(void* instance_struct, char *ptr, int is_manager, char *extra);

int init_reverse_shell(void* instance_struct);

char* get_name();    
int get_id();
void set_port(void* instance_struct, int portno);
int get_dat_siz();
char* get_agent_name(void* instance_struct);
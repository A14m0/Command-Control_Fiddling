#pragma once

#include "common.h"
#include "typedefs.h"

typedef struct _transport {
    // generic protocol functions for use in the main handler
    int (*send_ok)(void* instance_struct);
    int (*send_err)(void* instance_struct);
    int (*listen)(void* instance_struct);
    int (*read)(void* instance_struct, char **buff, int length);
    int (*write)(void* instance_struct, const char *buff, int length);

    int (*upload_file)(void* instance_struct, const char *ptr, int is_module);
    int (*init_reverse_shell)(void* instance_struct);
    int (*determine_handler)(void* instance_struct);
    
    int (*get_dat_siz)();
    int (*init)(void* instance_struct);
    int (*end)(void* instance_struct);

    char* (*get_name)();
    int (*get_id)();
    void (*set_port)(void* instance_struct, int portno);
    char* (*get_agent_name)(void* insatnce_struct);
} transport_t, *ptransport_t;


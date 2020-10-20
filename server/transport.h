#pragma once

// /#include "pthread.h"
#include "typedefs.h"

// defines error codes that can be returned by transport functions

#define API_OK 0
#define API_ERR_GENERIC 1
#define API_ERR_WRITE 2
#define API_ERR_READ 3
#define API_ERR_LISTEN 4
#define API_ERR_BIND 5
#define API_ERR_ACCEPT 6
#define API_ERR_AUTH 7
#define API_ERR_CLIENT 8
#define API_ERR_LOCAL 9


// defines structure of return data to be handled by all API calls

typedef struct _api_ret {
    int error_code;
    void *data;
} api_return, *papi_return;

typedef struct _transport {
    // generic protocol functions for use in the main handler
    api_return (*send_ok)(void* instance_struct);
    api_return (*send_err)(void* instance_struct);
    api_return (*listen)(void* instance_struct);
    api_return (*read)(void* instance_struct, char **buff, int length);
    api_return (*write)(void* instance_struct, const char *buff, int length);

    api_return (*upload_file)(void* instance_struct, const char *ptr, int is_module);
    api_return (*init_reverse_shell)(void* instance_struct);
    api_return (*determine_handler)(void* instance_struct);
    
    api_return (*get_dat_siz)();
    api_return (*init)(void* instance_struct);
    api_return (*end)(void* instance_struct);

    const char * (*get_name)();
    int (*get_id)();
    api_return (*set_port)(void* instance_struct, int portno);
    api_return (*get_agent_name)(void* insatnce_struct);
} transport_t, *ptransport_t;



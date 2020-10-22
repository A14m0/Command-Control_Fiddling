/* defines the transport API structure */
#pragma once
#include "common.h"

// defines module types
#define TRANSPORT 0
#define STANDALONE 1

// defines connection types
#define AGENT_TYPE 100
#define MANAG_TYPE 101

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


// structures that outlines transports
typedef struct _api_ret {
    int error_code;
    void *data;
} api_return, *papi_return;



class TransportAPI{
private:
    const int id;
    const char *name;

public:
    // gets the next available tasking
    // returns tasking struct with OP_NODATA if nothing available
    virtual api_return fetch_tasking();
    // pushes a task update to connected client
    virtual api_return push_tasking(ptask_t task);

    // starts listening for this instance
    virtual api_return listen();

    // returns transport name info
    const char *get_tname();
    // returns the currently connected agent's name
    virtual api_return get_aname();
    // returns transport ID
    int get_id();
    // sets the listening port of the instance
    virtual api_return set_port(int portno);
};


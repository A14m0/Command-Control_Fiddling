/* defines the transport API structure */
#pragma once
#include "types.h"

// defines module types
#define TRANSPORT 1
#define STANDALONE 2

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

// defines operation codes for tasking
#define OP_AUTH 0




class TransportAPI {
protected:
    int id;
    char *name;

public:
    // dummy constructors
    //virtual TransportAPI();
    //virtual ~TransportAPI();

    // gets the next available tasking
    // returns tasking struct with OP_NODATA if nothing available
    virtual api_return fetch_tasking() = 0;
    // pushes a task update to connected client
    virtual api_return push_tasking(ptask_t task) = 0;

    // starts listening for this instance
    virtual api_return listen() = 0;

    // returns transport name info
    virtual const char *get_tname() = 0;
    // returns the currently connected agent's name
    virtual api_return get_aname() = 0;
    // returns transport ID
    virtual int get_id() = 0;
    // sets the listening port of the instance
    virtual api_return set_port(int portno) = 0;
};


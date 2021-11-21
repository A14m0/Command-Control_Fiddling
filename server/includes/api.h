/* defines the transport API structure */
#pragma once
#include "types.h"
#include "values.h"


/// virtual class for defining a TransportAPI
class TransportAPI {
protected:
    int id;
    char *name;

public:
    virtual api_return fetch_tasking() = 0;             // gets the next available tasking
    virtual api_return push_tasking(ptask_t task) = 0;  // pushes a task update to connected client
    virtual api_return listen() = 0;                    // starts listening for this instance
    virtual const char *get_tname() = 0;                // returns transport name info
    virtual api_return get_aname() = 0;                 // returns the currently connected agent's name
    virtual int get_id() = 0;                           // returns transport ID
    virtual api_return set_port(int portno) = 0;        // sets the listening port of the instance
};


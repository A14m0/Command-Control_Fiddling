/* Defines generic server module class */
#pragma once

#include "common.h"
//#include "netinst.h"
#include "api.h"

class NetInst;

// Module class
class Module
{
private:
    const char *name; // name of the module
    int id; // ID of the module
    int type; // Type of the module (either transport or standalone)
    pthread_t thread; // thread refernce
    void *handle; // module handle
    void *(*generator)(NetInst *); // pointer to transport (null if standalone type)
    void (*entrypoint)(); // pointer to entrypoint (null if transport type)
    
public:
    Module(const char *name, const int id, 
                 const int type, void *handle, 
                 void (*entrypoint)(), void *(*generator)(NetInst *));
    ~Module();

    // gets module name
    const char *get_name();
    // gets module ID
    const int get_id();
    // gets module type
    const int get_type();
    // gets module entrypoint
    void *get_entrypoint();
    // gets modue api
    TransportAPI *new_transport(NetInst *);
    // sets module thread reference
    int set_thread(pthread_t thread);
    // closes dlsym handle
    void close_handle();

};
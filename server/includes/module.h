/* Defines generic server module class */
#pragma once

#include "common.h"

// Module class
class Module
{
private:
    const char *name; // name of the module
    int id; // ID of the module
    int type; // Type of the module (either transport or standalone)
    pthread_t thread; // thread refernce
    void *handle; // module handle
    ptransport_t *transport; // pointer to transport (null if standalone type)
    void (*entrypoint)(); // pointer to entrypoint (null if transport type)
    
public:
    Module(const char *name, const int id, 
                 const int type, void *handle, 
                 void (*entrypoint)(), ptransport_t *transport);
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
    ptransport_t *get_transport();
    // sets module thread reference
    int set_thread(pthread_t thread);
    // closes dlsym handle
    void close_handle();

};
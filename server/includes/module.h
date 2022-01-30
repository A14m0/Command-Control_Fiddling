/* Defines generic server module class */
#pragma once

#include "common.h"
#include "api.h"

// Module class
class Module
{
private:
    const char *name; // name of the module
    int id; // ID of the module
    int type; // Type of the module (either transport or standalone)
    void *handle; // module handle
    void *(*generator)(); // pointer to transport (null if standalone type)
    void (*entrypoint)(); // pointer to entrypoint (null if transport type)
    
public:
    Module(const char *name, const int id, 
                 const int type, void *handle, 
                 void (*entrypoint)(), void *(*generator)());
    ~Module();

    // gets module name
    const char *GetName();
    // gets module ID
    const int GetID();
    // gets module type
    const int GetType();
    // gets module entrypoint
    void *GetEntrypoint();
    // gets modue api
    TransportAPI *NewTransport();
    // closes dlsym handle
    void CloseHandle();

};
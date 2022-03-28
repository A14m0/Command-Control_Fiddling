/* Implements the Module class */
#include <dlfcn.h>
#include "module.h"


Module::Module(const char *name, const int id, 
                           const int type, void *handle, 
                           void (*entrypoint)(), void *(*generator)())
{
    this->name = name;
    this->id = id;
    this->type = type;
    this->handle = handle;
    this->entrypoint = entrypoint;
    this->generator = generator;
}

Module::~Module() {
    this->CloseHandle();
}

const char *Module::GetName(){
    return this->name;
}

const int Module::GetID(){
    return this->id;
}

const int Module::GetType(){
    return this->type;
}

void Module::CloseHandle(){
    dlclose(this->handle);
}

void *Module::GetEntrypoint(){
    return this->entrypoint;
}

TransportAPI *Module::NewTransport(){
    if(type == TRANSPORT){
        return (TransportAPI *)generator();
    }
    return nullptr;    
}
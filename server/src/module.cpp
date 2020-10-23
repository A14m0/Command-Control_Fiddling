/* Implements the Module class */
#include <dlfcn.h>
#include "module.h"


Module::Module(const char *name, const int id, 
                           const int type, void *handle, 
                           void (*entrypoint)(), void *(*generator)(NetInst *))
{
    this->name = name;
    this->id = id;
    this->type = type;
    this->handle = handle;
    this->entrypoint = entrypoint;
    this->generator = generator;
}

Module::~Module() {
    this->close_handle();
}

const char *Module::get_name(){
    return this->name;
}

const int Module::get_id(){
    return this->id;
}

const int Module::get_type(){
    return this->type;
}

int Module::set_thread(pthread_t thread){
    this->thread = thread;

    return 0;
}

void Module::close_handle(){
    dlclose(this->handle);
}

void *Module::get_entrypoint(){
    return this->entrypoint;
}

TransportAPI *Module::new_transport(NetInst *parent){
    if(type == TRANSPORT){
        return (TransportAPI *)generator(parent);
    }
    return nullptr;    
}
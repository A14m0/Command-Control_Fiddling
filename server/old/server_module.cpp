#include "server_module.h"


ServerModule::ServerModule(const char *name, const int id, 
                           const int type, void *handle, 
                           void (*entrypoint)(), ptransport_t *transport)
{
    this->name = name;
    this->id = id;
    this->type = type;
    this->handle = handle;
    this->entrypoint = entrypoint;
    this->transport = transport;
}

ServerModule::~ServerModule()
{
}

const char *ServerModule::get_name(){
    return this->name;
}

const int ServerModule::get_id(){
    return this->id;
}

const int ServerModule::get_type(){
    return this->type;
}

int ServerModule::set_thread(pthread_t thread){
    this->thread = thread;

    return 0;
}

void ServerModule::close_handle(){
    dlclose(this->handle);
}

void *ServerModule::get_entrypoint(){
    return this->entrypoint;
}

ptransport_t *ServerModule::get_transport(){
    return this->transport;
}
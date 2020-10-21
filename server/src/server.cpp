/* Define the server class here */
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>

#include "server.h"



Server::Server(){

    // initialize internal queues/vectors
    this->task_dispatch = new std::queue<ptask_t>();
    this->log_dispatch = new std::queue<plog_t>();
    this->modules = new std::vector<Module *>();


    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};


    // seed RNG
    srand(time(NULL));

    // gets current file path, so data will be written to correct folder regardless of where execution is called
	readlink( "/proc/self/exe", result, 4096);

	last = strrchr(result, '/');
	index = last - result;
	strncpy(dir, result, index);
	
    // change to executable's home directory 
	ret = chdir(dir);
	if(ret < 0){
		perror("Failed to change directory");
		exit(1);
	}

    // reset umask so we have full control of permissions
    umask(0);

    // initialize base directories
    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

    if (stat("out", &st) == -1) {
        mkdir("out", 0755);
        printf("Server: initialized directory 'out'\n");
    }

    if (stat("agents/agents.dat", &st) == -1) {
        int fd2 = open("agents/agents.dat", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }

    
    // populate backends
    if(!this->ReloadModules()){
        printf("[Server] Failed to load backends. Quitting...\n");
        exit(1);
    }
}

// writes/prints all logs
int Server::WriteLogs(){
    // loop over each log entry and log it
    while(!log_dispatch->empty()){
        DoLog(log_dispatch->front());
        log_dispatch->pop();
    }
    return 0;
}


// creates a new thread using module of id `id`
int Server::GenerateInstance(int id){
    bool found = false;
    // check if the ID exists in the list of modules
    for(Module *mod : *modules){
        if(mod->get_id() == id){
            found = true;
        }
    }

    if(!found){
        log(LOG_WARN, "Failed to find module of type '%d'", id);
        return 1;
    }

    // set up thread classes
    
    
    
    
    return 0;
}

// reloads the list of available modules
int Server::ReloadModules(){
    DIR *dir;
    struct dirent *ent;
    char *buff = (char*)malloc(2048);

    // clear the current vectors
    
    // close all currently open dl handles 
    for(Module *module : *(this->modules)){
        delete module;
    }
    this->modules->clear();

    // open modules directory 
    if ((dir = opendir ("shared/")) != NULL) {

        // loop over each module
        while ((ent = readdir (dir)) != NULL) {

            // ignore current and parent dir entries 
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {

                // get module path
                memset(buff, 0, 2048);
                sprintf(buff, "./shared/%s", ent->d_name);
                
                // open the module and handle the instance
                void *handle = dlopen(buff, RTLD_NOW);
                if(!handle) {
                    log(LOG_ERROR, "Failed to load .so file: %s\n", dlerror());    
                    continue;
                }

                AddModule(handle);
                printf("Added module to server\n");
            }
        }
    } else {
        // Failed to open directory 
        this->log(LOG_FATAL, "[Loader] Failed to open target directory 'shared/'\n");
        perror("");
        exit(2);
    }

    // close handle and return
    closedir (dir);
    return 0;
}

// adds a module to the server's known modules list
int Server::AddModule(void *handle){

    // check if the handle is not null
    if(!handle) {
        log(LOG_ERROR, "Broken handle found. Skipping...\n");
        return;
    }
    
    // get required global constants from the handle
    const int type = *(const int *)dlsym(handle, "type");
    const int id = *(const int *)dlsym(handle, "id");
    const char *name = *(const char **)dlsym(handle, "name");

    // check if they worked
    if(!type) {
        log(LOG_ERROR, "Failed to find type symbol\n");
        return;
    }
    
    if (!id){
        log(LOG_ERROR, "Failed to locate the module's ID\n");
        return 1;
    }
    if (!name){
        log(LOG_ERROR, "Failed to locate the module's name\n");
        return 1;
    }
            

    ptransport_t *api = new ptransport_t;
    void (*entrypoint)();
    Module *module;

    // handle the different module types 
    switch(type){
        case TRANSPORT:
            // resolve transport API and add it to available transport APIs
            *api = (ptransport_t)dlsym(handle, "transport_api");
            if(!api) {
                log(LOG_ERROR, "Failed to find transport API structure\n"); 
                return 1;
            }
            
            module = new Module(name, id, TRANSPORT, handle, 
                                      nullptr, api);
            break;

        case STANDALONE:
            // resolve entrypoint and add it to the available module_handles vector 
            entrypoint = (void (*)())dlsym(handle, "entrypoint");
            if (!entrypoint){
                log(LOG_ERROR, "Failed to locate the module's entrypoint function");
                return 1;
            }

            module = new Module(name, id, STANDALONE, handle, 
                                      entrypoint, nullptr);
            break;
        
        
        default:
            // unknown module type
            log(LOG_ERROR, "Unknown module type: %d\n", type);
            return 1;
    }

    // push the module
    this->modules->push_back(module);
    return 0;
}


// logs things
int Server::DoLog(plog_t log_ent){
    char *log_buffer = (char *)malloc(4096);
    memset(log_buffer, 0, 4096);
    
    char *reset = "\033[0m";
    char *concode = "";
    
    if(log_ent->message == NULL){
        log_ent->message = "NO INFO";
    }

    // set up format string and console colors
    switch (log_ent->type)
    {
    case LOG_INFO:
        concode = "\033[92m";
        sprintf(log_buffer, "[%d] [INFO]: %s", id, log_ent->message);
        break;
    case LOG_WARN:
        concode = "\033[93m";
        sprintf(log_buffer, "[%d] [WARNING]: %s", id, log_ent->message);
        break;
    case LOG_ERROR:
        concode = "\033[91m";
        sprintf(log_buffer, "[%d] [ERROR]: %s", id, log_ent->message);
        break;
    case LOG_FATAL:
        concode = "\033[1;31m";
        sprintf(log_buffer, "[%d] [FATAL]: %s", id, log_ent->message);
        break;
    default:
        break;
    }

    // check if `log_buffer` is null.
    // we don't do anything if we find one of them is null
    // because we want the program to crash if it fails heap allocations...
    if (log_buffer == NULL) {
        printf("Null buffr\n");
    }

    // print log
    printf(concode);
    printf(log_buffer);
    printf(reset);
    
    // write log here


    // free data
    free(log_buffer);

    return 0;
} 

// public function to push logs to the queue
int Server::PushLog(plog_t log_ent){
    log_dispatch->push(log_ent);
    return 0;
}


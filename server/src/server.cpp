/* Define the server class here */
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>

#include "server.h"
#include "netinst.h"



Server::Server(){

    // initialize internal queues/vectors
    this->task_dispatch = new std::deque<ptask_t>();
    this->log_dispatch = new std::queue<plog_t>();
    this->modules = new std::vector<Module *>();
    this->instances = new std::vector<NetInst *>();
    this->thread_objs = new std::vector<std::thread *>();


    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};

    memset(dir, 0, 4096);


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
        printf("Path: %s\n", dir);
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
    if(this->ReloadModules()){
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

// sends all available tasking to each registered NetInst
int Server::DispatchTasking(){
    // loop over each task
    while(!this->task_dispatch->empty()){
        printf("Size: %ld\n", this->task_dispatch->size());
        
        ptask_t task = this->task_dispatch->front();
        printf("server task ptr: %p\n", task);
        bool found = false;
        // loop over each NetInst
        for(NetInst *inst : *instances){
            // check for nullptr
            if(inst == nullptr){
                printf("FATAL\n");
                log(LOG_FATAL, "Instance has nullpointer! Exiting!!");
                exit(1);
            }
            if(inst->GetID() == task->to){
                inst->PushTasking(task);
                found = true;
            }
        }
        if(!found){
            log(LOG_ERROR, "No registered instance with ID '%d' found for tasking (type %d)", task->to, task->type);
        }
        printf("DEQUEING!\n");
        this->task_dispatch->pop_front();
    }
}


// creates a new thread using module of id `id`
int Server::GenerateInstance(int id){
    bool found = false;
    // check if the ID exists in the list of modules
    /*for(Module *mod : *modules){
        if(mod->GetID() == id){
            found = true;
            break;
        }
    }

    if(!found){
        log(LOG_WARN, "Failed to find module of type '%d'", id);
        return 1;
    }*/

    // set up thread classes
    int nid = rand();
    NetInst *inst = new NetInst(this, nid, NULL);
    instances->push_back(inst);
    std::thread *obj = inst->StartThread();
    thread_objs->push_back(obj);
    
    log(LOG_INFO, "Thread started with ID '%d'", nid);
    
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
                    log(LOG_ERROR, "Failed to load .so file: %s", dlerror());    
                    continue;
                }

                AddModule(handle);
            }
        }
    } else {
        // Failed to open directory 
        this->log(LOG_FATAL, "[Loader] Failed to open target directory 'shared/'");
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
        log(LOG_ERROR, "Broken handle found. Skipping...");
        return 1;
    }
    
    // get required global constants from the handle
    const int type = *(const int *)dlsym(handle, "type");
    const int id = *(const int *)dlsym(handle, "id");
    const char *name = *(const char **)dlsym(handle, "name");

    // check if they worked
    if(!type) {
        log(LOG_ERROR, "Failed to find type symbol");
        return 1;
    }
    
    if (!id){
        log(LOG_ERROR, "Failed to locate the module's ID");
        return 1;
    }
    if (!name){
        log(LOG_ERROR, "Failed to locate the module's name");
        return 1;
    }
            

    void *(*api_generator)(NetInst *);
    void (*entrypoint)();
    Module *module;

    // handle the different module types 
    switch(type){
        case TRANSPORT:
            // resolve transport API and add it to available transport APIs
            api_generator = (void *(*)(NetInst *))dlsym(handle, "generate_class");
            if(!api_generator) {
                log(LOG_ERROR, "Failed to find transport API structure"); 
                return 1;
            }
            
            module = new Module(name, id, TRANSPORT, handle, 
                                      nullptr, api_generator);
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
            log(LOG_ERROR, "Unknown module type: %d", type);
            return 1;
    }

    // push the module
    this->modules->push_back(module);
    return 0;
}


// logs things
int Server::DoLog(plog_t log_ent){
    char *log_buffer = (char *)malloc(4096);
    char time_str[4096];
    memset(log_buffer, 0, 4096);
    
    char *reset = "\033[0m";
    char *concode = "";

    time_t raw;
    time(&raw);
    struct tm *c_time;
    c_time = localtime(&raw);

    sprintf(time_str, "%i/%i/%02i  %i:%i:%02i", c_time->tm_mon+1, c_time->tm_mday, c_time->tm_year - 100, c_time->tm_hour, c_time->tm_min, c_time->tm_sec);
    
    if(log_ent->message == NULL){
        log_ent->message = "NO INFO";
    }

    // set up format string and console colors
    switch (log_ent->type)
    {
    case LOG_INFO:
        concode = "\033[92m";
        sprintf(log_buffer, "[%s] [%d] [INFO]: %s\n", time_str, log_ent->id, log_ent->message);
        break;
    case LOG_WARN:
        concode = "\033[93m";
        sprintf(log_buffer, "[%s] [%d] [WARNING]: %s\n",time_str, log_ent->id, log_ent->message);
        break;
    case LOG_ERROR:
        concode = "\033[91m";
        sprintf(log_buffer, "[%s] [%d] [ERROR]: %s\n", time_str, log_ent->id, log_ent->message);
        break;
    case LOG_FATAL:
        concode = "\033[1;4;5;31m";
        sprintf(log_buffer, "[%s] [%d] [FATAL]: %s\n", time_str, log_ent->id, log_ent->message);
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


// internal log function
// logs data to console and file
int Server::log(int type, const char *fmt, ...){
    char *log_buffer = (char *)malloc(4096);
    memset(log_buffer, 0, 4096);
    va_list vl;

    // format the format
    va_start(vl, fmt);
    vsprintf(log_buffer, fmt, vl);
    va_end(vl);


    // push log
    plog_t log_ent = (plog_t)malloc(sizeof(log_ent));

    log_ent->id = id;
    log_ent->type = type;
    log_ent->message = log_buffer;

    PushLog(log_ent);

    return 0;
} 

int Server::MainLoop(){

    /*
    UNIMPLEMENTED
    */

   GenerateInstance(0);

   ptask_t tmp = (ptask_t)malloc(sizeof(task_t));
   tmp->to = 0;
   tmp->from = 0;
   tmp->type = 0x1;
   tmp->length = 0;
   tmp->data = nullptr;

   PushTask(tmp);

   while(1){
        // print all accumulated logs
        WriteLogs();
        // dispatch tasking
        DispatchTasking();

        // check available tasking
        log(LOG_INFO, "Server Heartbeat");

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
   }


    return 0;
}

// public function to push logs to the queue
int Server::PushLog(plog_t log_ent){
    log_dispatch->push(log_ent);
    return 0;
}

// public function to push tasks to the queue
int Server::PushTask(ptask_t task){
    task_dispatch->push_back(task);
    return 0;
}
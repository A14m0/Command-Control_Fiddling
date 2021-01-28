/* Define the server class here */
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>


#define DEBUG 0
#define TRANSPORT_DIR "ssh_trans_new"
#define SRV_DELAY 200

#ifdef DEBUG
#include <assert.h>
#endif

#include "server.h"
#include "netinst.h"




Server::Server(){

    // initialize internal queues/vectors
    this->task_dispatch = new std::deque<ptask_t>();
    this->log_dispatch = new std::queue<plog_t>();
    this->modules = new std::vector<Module *>();
    this->instances = new std::vector<NetInst *>();
    this->thread_objs = new std::vector<std::thread *>();
    this->task_dispatch = new std::deque<ptask_t>();
    this->int_task_dispatch = new std::deque<ptask_t>();
    this->task_lock = PTHREAD_MUTEX_INITIALIZER;
    this->int_task_lock = PTHREAD_MUTEX_INITIALIZER;


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
    int loop = 0;
    pthread_mutex_lock(&task_lock);
    while(!this->task_dispatch->empty()){
        loop++;

        assert(loop <10);
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
                inst->ReceiveTasking(task);
                found = true;
            }
        }

        // check and see if its actually trying to go to the server
        // ID of zero is always the server
        if(task->to == 0){
            ReceiveTasking(task);
            found = true;
        }

        if(!found){
            log(LOG_ERROR, "No registered instance with ID '%d' found for tasking (type %d)", task->to, task->type);
        }
        printf("DEQUEING!\n");
        this->task_dispatch->pop_front();
    }

    pthread_mutex_unlock(&task_lock);

    //printf("Dispatch complete\n");

    return 0;
}


// creates a new thread using module of id `id`
int Server::GenerateInstance(Module* mod){
    //bool found = false;
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
    TransportAPI* transport = mod->NewTransport(inst);
    inst->SetTransport(transport);
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
    // note that we only look in ssh_trans_new for the time being
    // eventually we will progress to where the move to /shared makes sense
    // if ((dir = opendir ("shared/")) != NULL) {
    if ((dir = opendir (TRANSPORT_DIR)) != NULL) {

        // loop over each module
        while ((ent = readdir (dir)) != NULL) {

            // ignore non-lib entries 
            if(strncmp(ent->d_name, "lib", 3)){
                #ifdef DEBUG
                log(LOG_INFO, "Ignoring non-library file \"%s\"", ent->d_name);
                #endif
                continue;
            } else {

                // get module path
                memset(buff, 0, 2048);
                sprintf(buff, "./%s/%s", TRANSPORT_DIR, ent->d_name);
                
                // open the module and handle the instance
                void *handle = dlopen(buff, RTLD_NOW);
                if(!handle) {
                    log(LOG_ERROR, "Failed to load .so file: %s", dlerror());    
                    continue;
                }

                log(LOG_INFO, "Loading transport library \"%s\"", ent->d_name);

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
    const int id = *(const int *)dlsym(handle, "t_id");
    const char *name = *(const char **)dlsym(handle, "t_name");

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
    bool nodat = false;
    memset(log_buffer, 0, 4096);
    
    const char *reset = "\033[0m";
    char *concode;

    time_t raw;
    time(&raw);
    struct tm *c_time;
    c_time = localtime(&raw);

    sprintf(time_str, "%i/%i/%02i  %i:%i:%02i", c_time->tm_mon+1, c_time->tm_mday, c_time->tm_year - 100, c_time->tm_hour, c_time->tm_min, c_time->tm_sec);
    
    if(log_ent->message == NULL){
        log_ent->message = "NO INFO";
        nodat = true;
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
        concode = "";
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

    if(!nodat){
        free((void*)log_ent->message);
    }

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


// handles the adding of tasks to the internal task list
int Server::ReceiveTasking(ptask_t task){
    pthread_mutex_lock(&int_task_lock);
    int_task_dispatch->push_back(task);
    pthread_mutex_unlock(&int_task_lock);
    
    return 0;
}


// handles the server's tasking
int Server::HandleTaskings(){
    // unlike the NetInst, we lock everything before the while loop
    // because we want to minimize server soft locks

    pthread_mutex_lock(&int_task_lock);

    while(!int_task_dispatch->empty()){
        ptask_t task = int_task_dispatch->front();
        HandleTask(task);
        int_task_dispatch->pop_front();
    }

    pthread_mutex_unlock(&int_task_lock);
    return 0;
}


// handles an individual tasking operation
int Server::HandleTask(ptask_t task){
    printf("Task received:\n");
    printf("\tTO: %d\n", task->to);
    printf("\tFROM: %d\n", task->from);
    printf("\tTYPE: %d\n", task->type);
    printf("\tLENGTH: %lu\n", task->length);
    printf("\tDATA ADDRESS: %p\n", task->data);

    // free the task at the end
    FreeTask(task);

    return 0;
}


// frees a given task
void Server::FreeTask(ptask_t task){
    if(task->data == NULL){
        free(task->data);
    }
    free(task);
}


// the main logical loop of the server
int Server::MainLoop(){

    /*
    UNIMPLEMENTED
    */

   GenerateInstance(modules->back());

   ptask_t tmp = (ptask_t)malloc(sizeof(task_t));
   tmp->to = 0;
   tmp->from = 0;
   tmp->type = TASK_NULL;
   tmp->length = 0;
   tmp->data = nullptr;

   PushTask(tmp);

   while(1){
        // print all accumulated logs
        WriteLogs();

        // dispatch tasking
        DispatchTasking();

        // handle internal tasks first
        HandleTaskings();

        std::this_thread::sleep_for(std::chrono::milliseconds(SRV_DELAY));
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
    pthread_mutex_lock(&task_lock);
    task_dispatch->push_back(task);
    pthread_mutex_unlock(&task_lock);
    return 0;
}


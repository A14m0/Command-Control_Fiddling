/* implements NetInst class */
#include "netinst.h"
#include <inttypes.h>

// class contructor
NetInst::NetInst(Server *srv, int id, TransportAPI *transport){
    this->task_dispatch = new std::deque<ptask_t>();
    this->srv = srv;
    this->id = id;
    this->tspt = transport;
    this->int_task_lock = PTHREAD_MUTEX_INITIALIZER;
}

// class destructor
NetInst::~NetInst(){
    if(tspt){
        delete tspt;
    }
}

// main loop of the class
void NetInst::MainLoop(){
    log(LOG_INFO, "Beginning instance listen");

    // listen on the transport
    if(!api_check(tspt->listen())){
        printf("Failed");
        log(LOG_FATAL, "API CHECK FAILED ON TRANSPORT LISTEN!");
        return;
    }
    printf("Awaiting get_aname...\n");
    if(!api_check(tspt->get_aname())) {
        return;
    }
    // first thing we do is request from the server the manifest of the agent
    ptask_t test = CreateTasking(id, AGENT_SEND_BEACON, strlen((char*)(this->t_dat)), this->t_dat);
    PushTasking(test);


    while(1){
        printf("[NETINST] Within main thread loop\n");
        // handle tasks
        while(!task_dispatch->empty()){
            printf("Dequeueing...\n");
            pthread_mutex_lock(&int_task_lock);
            ptask_t task = task_dispatch->front();
            HandleTask(task);
            task_dispatch->pop_front();
            pthread_mutex_unlock(&int_task_lock);
        }
        
        //log(LOG_INFO, "This is the thread! WOO");
        //log(LOG_INFO, "Here's my second log!");

        
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
    }
    
    return;
}

// handles a given task
int NetInst::HandleTask(ptask_t task){
    printf("NetInst Task received:\n");
    printf("  TO: %d\n", task->to);
    printf("  FROM: %d\n", task->from);
    printf("  TYPE: %d\n", task->type);
    printf("  LENGTH: %lu\n", task->length);
    printf("  DATA ADDRESS: %p\n", task->data);
    int rc = 0;


    // switch depending on the type of operation
    switch (task->type)
    {
    case AGENT_DIE:
        printf("NETINST: Caught die\n");
        break;
    case AGENT_SLEEP:
        printf("NETINST: Caught sleep\n");
        break;
    case AGENT_DOWNLOAD_FILE:
        printf("NETINST: Caught download file\n");
        break;
    case AGENT_UPLOAD_FILE:
        printf("NETINST: Caught upload file\n");
        break;
    case AGENT_REVERSE_SHELL:
        printf("NETINST: Caught reverse shell\n");
        break;
    case AGENT_EXECUTE_SHELLSCRIPT:
        printf("NETINST: Caught execute shell command\n");
        break;
    case AGENT_SEND_BEACON:
        printf("NETINST: Caught beacon request\n");
        {
            // push request to agent and read data back
            if(!api_check(tspt->push_tasking(task))) {
                rc = 1;
                break;
            }
            if(!api_check(tspt->fetch_tasking())){
                rc = 1;
                break;
            }

            // get the beacon data
            char *beacon_data = (char*)((AgentJob*)this->t_dat)->get_data();
            // push data to the server
            ptask_t beacon = CreateTasking(0, TASK_PUSH_BEACON, strlen(beacon_data), beacon_data);
            PushTasking(beacon);
            printf("[NETINST] Awaiting task...\n");
        }
        break;
    default:
        log(LOG_ERROR, "Unknown tasking operation: %d", task->type);
        break;
    }

    // free the task at the end
    FreeTask(task);
    
    return rc;
}

// frees a given task
void NetInst::FreeTask(ptask_t task){
    if(task->data == NULL){
        free(task->data);
    }
    free(task);
}


// logs data to console and file
int NetInst::log(int type, const char *fmt, ...){
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

    srv->PushLog(log_ent);

    return 0;
} 


// wrapper for creating a ptask_t structure in heap
ptask_t NetInst::CreateTasking(int to, unsigned char type, unsigned long length, void *data){
    ptask_t ret = (ptask_t)malloc(sizeof(task_t));
    memset(ret, 0, sizeof(task_t));
    ret->to = to;
    ret->from = id;
    ret->type = type;
    ret->length = length;

    // sanity check to make sure the data is actually on the heap
    // if it is not, then we allocate for it and copy everything 
    uintptr_t test1 = ((uintptr_t)ret)&0xffff00000000;
    uintptr_t test2 = ((uintptr_t)data)&0xffff00000000;
    //printf("%" PRIxPTR "\n", test1);
    //printf("%" PRIxPTR "\n", test2);
    if (test1 == test2) {
        log(LOG_ERROR, "Task data outside heap, moving to heap...");
        void *dat = malloc(length);
        memcpy(dat, data, length);
        uintptr_t test3 = ((uintptr_t)dat)&0xffff00000000;
        if (test1 != test3) {
            printf("Failed the thing....\n");
            printf("%p %p %p\n", ret, data, dat);
            exit(1);
        } else {
            ret->data = dat;
        }
    } else {
        ret->data = data;
    }
    
    return ret;
}


// pushes a tasking struct to the server
int NetInst::PushTasking(ptask_t task){
    srv->PushTask(task);
    return 0;
}

// lets others push tasking to this instance
int NetInst::ReceiveTasking(ptask_t task){
    pthread_mutex_lock(&int_task_lock);
    task_dispatch->push_back(task);
    pthread_mutex_unlock(&int_task_lock);
    return 0;
}


// awaits a particular tasking type and returns it when found
ptask_t NetInst::AwaitTask(int type){
    while(1){
        // loop over each item in the queue and see if its of the type we want
        for(ptask_t task : *task_dispatch){
            if(task->type == type){
                return task;
            } 
        }
        // wait before updating
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
} 

// returns the ID of this instance
int NetInst::GetID(){
    return id;
}



// Handles response from API calls 
bool NetInst::api_check(api_return api){
    char *info = "[NO INFO]";
    char def[10];
    char *err_type = "";

    if(api.data != NULL){
        info = (char*)api.data;
    } 

    switch (api.error_code)
    {
    case API_OK:
        if(api.data != NULL){
            this->t_dat = api.data;
        }
        return true;
        break;

    case API_ERR_GENERIC:
        err_type = "generic";
        break;
    case API_ERR_WRITE:
        err_type = "write";
        break;
    case API_ERR_READ:
        err_type = "read";
        break;
    case API_ERR_LISTEN:
        err_type = "socket listen";
        break;
    case API_ERR_BIND:
        err_type = "port bind";
        break;
    case API_ERR_ACCEPT:
        err_type = "socket accept";
        break;
    case API_ERR_AUTH:
        err_type = "authentication";
        break;
    case API_ERR_CLIENT:
        err_type = "client-side";
        break;
    case API_ERR_LOCAL:
        err_type = "server-side";
        break;
    default:
        err_type = "unknown API";
        info = def;
        sprintf(info, "%d", api.error_code);
        break;
    }

    this->log(LOG_ERROR, "API encountered %s error: %s (%s)", "GENERIC", err_type, info);
    return false;
}

// trampoline p2
void NetInst::Trampoline(void *self){
    ((NetInst *)(self))->MainLoop();
}

// trampoline to thread execution
std::thread *NetInst::StartThread(){
    std::thread *thread_obj = new std::thread(NetInst::Trampoline, this);
    return thread_obj;
}

void NetInst::SetTransport(TransportAPI *transport){
    this->tspt = transport;
}
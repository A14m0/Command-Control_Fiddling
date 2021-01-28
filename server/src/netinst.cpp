/* implements NetInst class */
#include "netinst.h"


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
    
    /*
        UNIMPLEMENTED
    */

   if(!api_check(tspt->listen())){
       return;
   }

    while(1){
        // handle tasks
        while(!task_dispatch->empty()){
            pthread_mutex_lock(&int_task_lock);
            ptask_t task = task_dispatch->front();
            HandleTask(task);
            task_dispatch->pop_front();
            pthread_mutex_unlock(&int_task_lock);
        }
        
        log(LOG_INFO, "This is the thread! WOO");
        log(LOG_INFO, "Here's my second log!");

        ptask_t test = CreateTasking(id, TASK_AUTH, 0, nullptr);
        printf("Thread task ptr: %p\n", test);
        PushTasking(test);

        std::this_thread::sleep_for(std::chrono::seconds(5));
        
    }
    
    return;
}

// handles a given task
int NetInst::HandleTask(ptask_t task){
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
    ret->data = data;
    
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
        // loop over each item in the queue and see if its of the type we wwant
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

    this->log(LOG_ERROR, "API encountered %d error: %s", "GENERIC", err_type, info);
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
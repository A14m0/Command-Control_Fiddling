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

NetInst::NetInst(Server *srv, int id, Module *mod) {
    this->task_dispatch = new std::deque<ptask_t>();
    this->srv = srv;
    this->id = id;
    this->tspt = mod->NewTransport();
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
    // log(LOG_INFO, "Beginning instance listen");
    int rc = 0, exit_code = 0;
    int fail_count = 0;

    // listen on the transport
    if(!api_check(tspt->listen())){
        log(LOG_FATAL, "API CHECK FAILED ON TRANSPORT LISTEN!\n");
        log(LOG_INFO, "Terminating network instance...\n");
        ptask_t die = CreateTasking(id, NETINST_TERMINATE, 0, nullptr);
        PushTasking(die);
        return;
    }
    
    if(!api_check(tspt->get_aname())) {
        ptask_t die = CreateTasking(id, NETINST_TERMINATE, 0, nullptr);
        PushTasking(die);
        return;
    }
    if(!this->t_dat){
        log(LOG_FATAL, "AGENT NAME IS NULL! Suiciding network instance...\n");
        ptask_t die = CreateTasking(id, NETINST_TERMINATE, 0, nullptr);
        PushTasking(die);
        return;
    }
    // first thing we do is request from the server the manifest of the agent
    ptask_t test = CreateTasking(id, AGENT_SEND_BEACON, strlen((char*)(this->t_dat)), this->t_dat);
    PushTasking(test);


    while(1){
        // handle tasks
        while(!task_dispatch->empty()){
            pthread_mutex_lock(&int_task_lock);
            ptask_t task = task_dispatch->front();
            rc = HandleTask(task);
            printf("RC: %d, fail count: %d\n", rc);
            if(rc == 2 || fail_count > 5) {
                exit_code = 1;
                break;
            } else if (rc == 1) {
                fail_count++;
            } else {
                fail_count = 0;
            }
            task_dispatch->pop_front();
            pthread_mutex_unlock(&int_task_lock);
        }

        // check to see if we died during the execution
        if(!exit_code) {
            // generate a heartbeat task
            ptask_t test = CreateTasking(id, AGENT_HEARTBEAT, strlen((char*)(this->t_dat)), this->t_dat);
            PushTasking(test);

            std::this_thread::sleep_for(std::chrono::seconds(5));
        } else {
            log(LOG_INFO, "Terminating network instance...\n");
            ptask_t die = CreateTasking(id, NETINST_TERMINATE, 0, nullptr);
            PushTasking(die);
        }
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

    printf("Malloc address: %p\n", malloc);


    // switch depending on the type of operation
    switch (task->type)
    {
    case AGENT_SLEEP:
    case AGENT_DIE:
        printf("NETINST: Caught die\n");
        {
            // push the die, and suicide the connection
            if(!api_check(tspt->push_tasking(task))) {
                log(LOG_ERROR, "NETINST AGENT_DIE: Failed API check push tasking\n");
                rc = 1;
                break;
            }

            delete tspt;
            rc = 2; 
            break;
        }
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
    case AGENT_HEARTBEAT:
        printf("NETINST: Caught beacon/heartbeat request\n");
        {
            // push request to agent and read data back
            if(!api_check(tspt->push_tasking(task))) {
                printf("Failed api check push\n");
                rc = 1;
                break;
            }
            printf("Fetching task\n");
            if(!api_check(tspt->fetch_tasking())){
                printf("Failed api check fetch task\n");
                rc = 1;
                break;
            }
            printf("T\n");
            // get the beacon data
            char *beacon_data = (char*)((AgentJob*)this->t_dat)->get_data();
            if(beacon_data == nullptr) {
                log(LOG_ERROR, "Task data was null (AGENT_BEACON/HEARTBEAT)\n");
                break;
            }
            // push data to the server
            ptask_t beacon = CreateTasking(0, TASK_PUSH_BEACON, strlen(beacon_data), beacon_data);
            PushTasking(beacon);
            //printf("[NETINST] Awaiting task...\n");
        }
        break;

    // Manager tasking operations
    case MANAGER_RETRIEVE_AGENT:
        printf("Caught manager beacon fetch\n");
        break;
    case MANAGER_RETRIEVE_LOOT:
        printf("Caught manager loot fetch\n");
        break;
    case MANAGER_UPLOAD_FILE:
        printf("Caught manager file upload\n");
        break;
    case MANAGER_DOWNLOAD_FILE:
        printf("Caught manager file download\n");
        break;
    case MANAGER_PUSH_MODULE:
        printf("Caught manager module upload\n");
        break;
    case MANAGER_RUN_COMMAND:
        printf("Caught manager command string\n");
        break;
    case MANAGER_REQUEST_REVERSESHELL:
        printf("Caught manager reverse shell request\n");
        break;
    case MANAGER_REGISTER_AGENT:
        printf("Caught manager agent registration\n");
        break;
    case MANAGER_REVIEW_TRANSPORTS:
        printf("Caught manager transport list\n");
        break;
    case MANAGER_START_TRANSPORT:
        printf("Caught manager transport start\n");
        break;
    case MANAGER_EXIT:
        printf("Caught manager termination\n");
        break;
    default:
        log(LOG_ERROR, "Unknown tasking operation: %d\n", task->type);
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
    //uintptr_t test1 = ((uintptr_t)ret)&0xffff00000000;
    //uintptr_t test2 = ((uintptr_t)data)&0xffff00000000;
    //printf("%" PRIxPTR "\n", test1);
    //printf("%" PRIxPTR "\n", test2);
    /*if (test1 == test2) {
        log(LOG_ERROR, "Task data outside heap, moving to heap...");
        void *dat = malloc(length);
        memcpy(dat, data, length);
        uintptr_t test3 = ((uintptr_t)dat)&0xffff00000000;
        if (test1 != test3) {
            printf("Failed the thing....\n");
            printf("%p %p %p\n", ret, data, dat);
            perror("Reason for failure");
            exit(1);
        } else {
            ret->data = dat;
        }
    } else {*/
        ret->data = data;
    //}
    
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

// returns the agent's identifier
char* NetInst::GetAgentName() {
    if(!api_check(tspt->get_aname())){
        log(LOG_ERROR, "Failed to get agent's name\n");
        return nullptr;
    } else {
        return (char*)t_dat;
    }
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

// sets the transport of the structure
void NetInst::SetTransport(TransportAPI *transport){
    this->tspt = transport;
}

// sets the port on the transport
int NetInst::SetTransportPort(int portno) {
    if(!api_check(tspt->set_port(portno))) {
        return 1;
    }
    return 0;
}



////////////////// THREAD TRAMPOLINE //////////////////////////

// trampoline p2
void NetInst::Trampoline(void *self){
    ((NetInst *)(self))->MainLoop();
}

// trampoline to thread execution
std::thread *NetInst::StartThread(){
    std::thread *thread_obj = new std::thread(NetInst::Trampoline, this);
    return thread_obj;
}

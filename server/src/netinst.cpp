/* implements NetInst class */
#include "netinst.h"


// class contructor
NetInst::NetInst(Server *srv, int id, ptransport_t transport){
    this->srv = srv;
    this->id = id;
    this->tspt = transport;
}

// class destructor
NetInst::~NetInst(){
    if(!api_check(tspt->end(t_dat))){
        log(LOG_ERROR, "Failed to terminate transport! Force-killing...");
    }
}


// handles a given task
int NetInst::HandleTask(task_t task){
    /*
        UNIMPLEMENTED
    */
    
    return 0;
}

// frees a given task
void NetInst::FreeTask(ptask_t task){
    free(task->data);
    free(task);
}


// logs data to console and file
int NetInst::log(int type, char *fmt, ...){
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

    this->log(LOG_ERROR, "API encountered %d error: %s\n", "GENERIC", err_type, info);
    return false;
}


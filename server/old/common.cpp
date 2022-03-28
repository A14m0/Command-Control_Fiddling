#include "common.h"

Common::Common(){
    this->logger = new Log();
}

Common::~Common(){

}

/*Common logging function that both prints data 
and writes data to log file*/
int Common::log(const char *format, const char *id, ...){
    char *logbuff = (char*)malloc(2048);
    char frmt[2048];
    memset(logbuff, 0, 2048);
    memset(frmt, 0, 2048);
    va_list args;

    sprintf(frmt, "[%s] ", id);
    strcat(frmt, format);

    
    va_start(args, id);
    vsnprintf(logbuff, 2048, frmt, args);
    va_end(args);

    return this->logger->log(logbuff);
}

/* Handles response from API calls */
bool Common::api_check(api_return api){
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
            this->api_data = api.data;
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

    this->log("API encountered %d error: %s\n", "GENERIC", err_type, info);
    return false;
}
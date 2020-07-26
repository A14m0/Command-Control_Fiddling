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
    switch (api.error_code)
    {
    case API_OK:
        if(api.data != NULL){
            this->api_data = api.data;
        }
        return true;
        break;

    case API_ERR_GENERIC:
        if(api.data != NULL){
            this->log("API encountered a generic error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a generic error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_WRITE:
        if(api.data != NULL){
            this->log("API encountered a write error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a write error: [NO INFO]\n", "GENERIC");
        }
        break;
    
    case API_ERR_READ:
        if(api.data != NULL){
            this->log("API encountered a read error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a read error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_LISTEN:
        if(api.data != NULL){
            this->log("API encountered a listening error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a listening error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_BIND:
        if(api.data != NULL){
            this->log("API encountered a port bind error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a port bind error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_ACCEPT:
        if(api.data != NULL){
            this->log("API encountered a socket accept error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a socket accept error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_AUTH:
        if(api.data != NULL){
            this->log("API encountered an authentication error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered an authentication error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_CLIENT:
        if(api.data != NULL){
            this->log("API encountered a client-side error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a client-side error: [NO INFO]\n", "GENERIC");
        }
        break;

    case API_ERR_LOCAL:
        if(api.data != NULL){
            this->log("API encountered a local error: %s\n", "GENERIC", api.data);
        } else {
            this->log("API encountered a local error: [NO INFO]\n", "GENERIC");
        }
        break;

    default:
        this->log("Caught unknown API error code: %d\n", "GENERIC", api.error_code);
        break;
    }
    return false;
}
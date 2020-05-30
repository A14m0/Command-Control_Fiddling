#include "common.h"

Common::Common(){
    this->logger = new Log();
}

Common::~Common(){

}


int Common::log(const char *format, char *id, ...){
    char *logbuff = (char*)malloc(2048);
    memset(logbuff, 0, sizeof(logbuff));
    va_list args;

    va_start(args, id);
    snprintf(logbuff, sizeof(logbuff), format, id, args);
    va_end(args);

    this->logger->log(logbuff);
}
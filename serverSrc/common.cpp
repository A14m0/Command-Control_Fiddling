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
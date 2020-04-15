#pragma once

#include "common.h"
#include <stdarg.h>

class Log
{
private:
    FILE *logfile;
    pthread_mutex_t session_lock;
public:
    Log(/* args */);
    ~Log();
    int log(const char *format, char *id, ...);
};
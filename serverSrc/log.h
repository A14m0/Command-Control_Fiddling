#pragma once

#include "common.h"
#include <stdarg.h>

class Log
{
private:
    FILE *logfile;
    pthread_mutex_t session_lock;
public:
    Log();
    Log(pthread_mutex_t lock);
    ~Log();
    int log(const char *format, char *id, ...);
    pthread_mutex_t get_mutex();
    int open_log();
    int close_log();
};
#pragma once

#include "common.h"

class Log
{
private:
    FILE *logfile;
    pthread_mutex_t session_lock;
public:
    Log(/* args */);
    ~Log();
    int log(char *dat);
};
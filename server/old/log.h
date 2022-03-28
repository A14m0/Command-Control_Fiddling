#pragma once

#include "common.h"

class Log
{
private:
    FILE *logfile;
public:
    Log();
    ~Log();
    int log(char *buffer);
    int open_log();
    int close_log();
};
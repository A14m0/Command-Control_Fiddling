/* Defines common inherited attributes/functions of all classes */
#pragma once

#include <queue>
#include <thread>
#include <chrono>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
//#include "api.h"
#include "log.h"
#include "values.h"

// task structure passed to dispatch queues
typedef struct _task {
    int to; // defines what ID the task is meant for
    int from; // defines who sent the task
    unsigned char type; // OPCODE of the task
    unsigned long length; // length of data
    void *data; // pointer to data on heap
} task_t, *ptask_t;

// defines task type vales
#define TASK_AUTH 0x1
#define TASK_NEW_NETINST 0x2
#define TASK_WRITE_BEACON 0x3


// log structure used to pass logs from threads to server
typedef struct _log {
    int id; // who sent the log
    int type; // log type
    const char *message; // message of the log 
} log_t, *plog_t;


// common class
class Common
{
protected:
    int id; // class ID
    
    // dispatch queue
    std::deque<ptask_t> *task_dispatch;

    //virtual int PushTask(ptask_t);
    //virtual ptask_t FetchDispatch();
    //virtual int log(int log_type, char *format, ...);
    
public:
    Common();
    ~Common();
};


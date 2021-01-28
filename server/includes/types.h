#pragma once

// task structure passed to dispatch queues
typedef struct _task {
    int to; // defines what ID the task is meant for
    int from; // defines who sent the task
    unsigned char type; // OPCODE of the task
    unsigned long length; // length of data
    void *data; // pointer to data on heap
} task_t, *ptask_t;


// authentication credentials helper structure
typedef struct ret
{
    char *usr;
    char *passwd;
} passwd_t, *ppasswd_t;


// log structure used to pass logs from threads to server
typedef struct _log {
    int id; // who sent the log
    int type; // log type
    const char *message; // message of the log
} log_t, *plog_t;


// structures that outlines transports
typedef struct _api_ret {
    int error_code;
    void *data;
} api_return, *papi_return;
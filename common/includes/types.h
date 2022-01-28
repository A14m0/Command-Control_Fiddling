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
typedef struct _passwd_t
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


// defines authentication structures
typedef struct _auth {
    char uname[128];
    char passwd[64];
} auth_t, *pauth_t;


// define our networked file data structure
typedef struct _net_file {
    unsigned long fsize;    // the length of the file
    unsigned int psize;     // the length of the path string
    char *path;             // path to save the file to (can be null for in-memory)
    void *data;             // raw data of the file
} net_file, *pnet_file;
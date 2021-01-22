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
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/sha.h>

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


// authentication credentials helper structure
typedef struct ret
{
    char *usr;
    char *passwd;
} passwd_t, *ppasswd_t;

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
    static int index_of(const char* str, const char find, int rev);
    static int directory_exists(const char* path);
    static void clean_input(char *input);
    static char* substring(const char* string, int position);
    static char** str_split(char* str, const char delim);
    static int init_agent(const char* agent_id);
    static int _register_agent(const char* username, const char* password);
    static int register_agent(char* line);
    static char* get_agent_tasking(const char* agent_id);
    static ppasswd_t gen_agent_creds();
    static int task_agent(const int operation, const char* agent, const char* opt);
    static int write_agent_beacon(const char* id, const char* beacon);
    static int write_default_agent_manifest(char* path);
    static char* digest(const char* input);
    ~Common();
};

// base64 encoding/decoding functions
class B64
{
private:
    /* data */
public:
    static size_t enc_size(size_t inlen);
    static size_t dec_size(const char *in);
    static void encode(const unsigned char *in, size_t len, char **buff);
    static int decode(const char *in, unsigned char *out, size_t outlen);
    static int isvalidchar(char c);
    static const char b64chars[];
    static const int b64invs[];
};


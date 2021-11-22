/* Defines common inherited attributes/functions of all classes */
#pragma once

// include system libraries
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

// our own headers
#include "log.h"
#include "values.h"
#include "types.h"



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
    static int get_file(const char* filename, char **buff);
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


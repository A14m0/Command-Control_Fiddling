#pragma once

#include "common.h"

#define MANAGE_STATE 3
#define AGENT_STATE 2
#define ACTIVE_STATE 1
#define AWAIT_STATE 0

typedef struct _avail_sess {
    int session_id;
    char name[20];
    void *shared_mem;
} avail_sess, *p_avail_sess;

class ShellComms
{
private:
    int state = AWAIT_STATE;
    p_avail_sess session;
    pthread_mutex_t memlock;
public:
    ShellComms(/* args */);
    ~ShellComms();
    int get_id();
    char *get_name();
    int start_session();
    int wait_read(void** buffer, int len, int flag);
    int wait_write(void *buffer, int len, int flag);
    bool check_state(int target_state);
};


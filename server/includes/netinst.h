/* Defines individual communication instance class */
#pragma once

#include "common.h"
#include "server.h"
#include "api.h"
#include "tasking.h"

class NetInst : Common
{
private:
    // instance data for transports
    void *t_dat;
    TransportAPI *tspt;
    Server *srv;
    pthread_mutex_t int_task_lock;

    void MainLoop();
    int HandleTask(ptask_t task);
    void FreeTask(ptask_t task);
    bool api_check(api_return api);
    
    static void Trampoline(void*self);

public:
    int log(int type, const char *frmt, ...);// override;
    NetInst(Server *server, int id, TransportAPI *transport); 

    std::thread *StartThread();
    ptask_t CreateTasking(int to, unsigned char type, unsigned long length, void *data);
    int PushTasking(ptask_t task);
    int ReceiveTasking(ptask_t task);
    void SetTransport(TransportAPI *transport);
    ptask_t AwaitTask(int type); 
    void *Malloc(int size);

    int GetID();

    ~NetInst();
};


/* Defines individual communication instance class */
#pragma once

#include "common.h"
#include "server.h"

class NetInst : Common
{
private:
    // instance data for transports
    void *t_dat;
    TransportAPI *tspt;
    Server *srv;

    void MainLoop();
    int HandleTask(task_t task);
    void FreeTask(ptask_t task);
    bool api_check(api_return api);
    
    static void Trampoline(void*self);

public:
    int log(int type, char *frmt, ...);// override;
    NetInst(Server *server, int id, TransportAPI *transport); 

    std::thread *StartThread();
    ptask_t AwaitTask(int type); 

    ~NetInst();
};


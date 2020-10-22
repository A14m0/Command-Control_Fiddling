/* Defines individual communication instance class */
#pragma once

#include "common.h"
#include "server.h"

class NetInst : Common
{
private:
    // instance data for transports
    void *t_dat;
    ptransport_t tspt;
    Server *srv;

    void MainLoop();
    int HandleTask(task_t task);
    void FreeTask(ptask_t task);
    bool api_check(api_return api);
    int log(int type, char *frmt, ...);// override;

    static void Trampoline(void*self);

public:
    // constructs a CommInst
    NetInst(Server *server, int id, ptransport_t transport); 

    std::thread *StartThread();

    ~NetInst();
};


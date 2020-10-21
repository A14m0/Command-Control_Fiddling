/* Defines individual communication instance class */
#include "common.h"
#include "server.h"

class NetInst : Common
{
private:
    /* data */
public:
    // constructs a CommInst
    NetInst(Server *server, int id, ptransport_t transport); 

    int HandleTask(task_t task);
    void FreeTask(task_t task);

    ~NetInst();
};


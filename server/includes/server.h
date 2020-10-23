/* Server class header */
#pragma once

#include <vector>
#include "common.h"
#include "module.h"

// struct used when creating a new instance
typedef struct _instance {
    int id;
    std::queue<ptask_t> *dispatch;
} instance_t, *pinstance_t;


// server class
class Server : Common
{
private:
    int id = 0; // Server ID
    std::queue<plog_t> *log_dispatch; // log queue
    std::vector<Module *> *modules; // list of available transports
    std::vector<NetInst *> *instances; // list of registered instances
    std::vector<std::thread *> *thread_objs; // thread object memory reference

    int WriteLogs(); // loops over all available logs and writes/prints them
    int DispatchTasking();
    int GenerateInstance(int id); // generates and registers a new comms instance
    int ReloadModules(); // clears and repopulates internal transports vector
    int AddModule(void *handle); // adds a module to the internal modules vector

    int DoLog(plog_t log_ent); // writes a plog_t to console/file
    int log(int type, const char *fmt, ...);// override; // internal logging function
public:
    Server(/* args */);
    ~Server();

    int PushLog(plog_t log); // pushes log to log_dispatch queue
    int PushTask(ptask_t task); // pushes task to task_dispatch queue

    int MainLoop();
};


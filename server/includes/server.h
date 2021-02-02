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
    pthread_mutex_t task_lock; // task thread lock
    pthread_mutex_t int_task_lock; // internal task thread lock
    std::queue<plog_t> *log_dispatch; // log queue
    std::vector<Module *> *modules; // list of available transports
    std::vector<NetInst *> *instances; // list of registered instances
    std::vector<std::thread *> *thread_objs; // thread object memory reference
    std::deque<ptask_t> *task_dispatch; // list of tasks for the server
    std::deque<ptask_t> *int_task_dispatch; // internal task dispatch queue

    int WriteLogs(); // loops over all available logs and writes/prints them
    int DispatchTasking();
    int GenerateInstance(Module* mod); // generates and registers a new comms instance
    int ReloadModules(); // clears and repopulates internal transports vector
    int AddModule(void *handle); // adds a module to the internal modules vector
    int ReceiveTasking(ptask_t task); // adds a task to internal list
    int HandleTaskings(); // handles all taskings assigned to server
    int HandleTask(ptask_t task); // handles an individual task
    int Authenticate(pauth_t auth); // checks agent passphrase and username against known agents
    void FreeTask(ptask_t task); // safely deletes a task object
    ptask_t CreateTasking(int to, unsigned char type, unsigned long length, void *data); // generates a tasking given the input

    int DoLog(plog_t log_ent); // writes a plog_t to console/file
    int log(int type, const char *fmt, ...);// override; // internal logging function
public:
    Server(/* args */);
    ~Server();

    int PushLog(plog_t log); // pushes log to log_dispatch queue
    int PushTask(ptask_t task); // pushes task to task_dispatch queue

    int MainLoop();
};


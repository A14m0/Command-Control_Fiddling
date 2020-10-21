/* Server class header */
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

    int WriteLogs(); // loops over all available logs and writes/prints them
    int GenerateInstance(int id); // generates and registers a new comms instance
    int ReloadModules(); // clears and repopulates internal transports vector
    int AddModule(void *handle); // adds a module to the internal modules vector

    int DoLog(plog_t log_ent); // writes a plog_t to console/file
    int log(int type, char *fmt, ...) override; // internal logging function
public:
    Server(/* args */);
    ~Server();

    int PushLog(plog_t log); // pushes log to log_dispatch queue
    
};


/* Server class header */
#include <vector>
#include "common.h"

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
    std::vector<ptransport_t> *transports; // list of available transports

    int WriteLogs(); // loops over all available logs and writes/prints them
    int GenerateInstance(); // generates and registers a new comms instance
public:
    Server(/* args */);
    ~Server();

    int PushLog(plog_t log); // pushes log to log_dispatch queue
};


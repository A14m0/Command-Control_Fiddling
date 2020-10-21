/* Defines common inherited attributes/functions of all classes */
#include <queue>
#include <stdarg.h>


// task structure passed to dispatch queues
typedef struct _task {
    int to; // defines what ID the task is meant for
    int from; // defines who sent the task
    unsigned char type; // OPCODE of the task
    unsigned long length; // length of data
    void *data; // pointer to data on heap
} task_t, *ptask_t;

// log structure used to pass logs from threads to server
typedef struct _log {
    int id; // who sent the log
    int type; // log type
    const char *message; // message of the log 
} log_t, *plog_t;

// structures that outlines transports
typedef struct _api_ret {
    int error_code;
    void *data;
} api_return, *papi_return;
//////////////// NOTE: FUNCTIONS TBD ///////////////////////////////////////
typedef struct _transport {
    // generic protocol functions for use in the main handler
    api_return (*send_ok)(void* instance_struct);
    api_return (*send_err)(void* instance_struct);
    api_return (*listen)(void* instance_struct);
    api_return (*read)(void* instance_struct, char **buff, int length);
    api_return (*write)(void* instance_struct, const char *buff, int length);

    api_return (*upload_file)(void* instance_struct, const char *ptr, int is_module);
    api_return (*init_reverse_shell)(void* instance_struct);
    api_return (*determine_handler)(void* instance_struct);
    
    api_return (*get_dat_siz)();
    api_return (*init)(void* instance_struct);
    api_return (*end)(void* instance_struct);

    const char * (*get_name)();
    int (*get_id)();
    api_return (*set_port)(void* instance_struct, int portno);
    api_return (*get_agent_name)(void* insatnce_struct);
} transport_t, *ptransport_t;
////////////////////////////////////////////////////////////////////////////

// common class
class Common
{
private:
    int id; // class ID

    // dispatch queue
    std::queue<ptask_t> *dispatch;

    virtual int PushTask(ptask_t);
    virtual ptask_t FetchDispatch();
    virtual int log(int log_type, char *format, ...);
    bool api_check(api_return api);
    
public:
    
    
};


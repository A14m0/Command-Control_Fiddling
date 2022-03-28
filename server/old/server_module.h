#include "common.h"
#include "typedefs.h"
#include <dlfcn.h>

class ServerModule
{
private:
    const char *name;
    int id;
    int type;
    pthread_t thread;
    void *handle;
    ptransport_t *transport;
    void (*entrypoint)();
    
public:
    ServerModule(const char *name, const int id, 
                 const int type, void *handle, 
                 void (*entrypoint)(), ptransport_t *transport);
    ~ServerModule();

    const char *get_name();
    const int get_id();
    const int get_type();
    void *get_entrypoint();
    ptransport_t *get_transport();

    int set_thread(pthread_t thread);

    void close_handle();

};

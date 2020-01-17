#include "transport.h"
#include "log.h"
#include "list.h"
#include "authenticate.h"

class Server
{
private:
    class Log *logger;
    class List *list;
public:
    Server();
    ~Server();

    // fetch functions
    void get_info(class ServerTransport *transport, char *ptr);
    void get_ports(class ServerTransport *transport, char *ptr);
    void get_loot(class ServerTransport *transport);

    // handler functions
    void *handle_connection(void *input);
    void authenticate(void *sess);
    void manager_handler(class ServerTransport *transport);
    void agent_handler(class ServerTransport *transport);
    
    // file functions
    void upload_file(class ServerTransport *transport, const char *path, int is_module);
    void download_file(class ServerTransport *transport, char *d_ptr, int op, char *dat_ptr);
    void reverse_shell(class ServerTransport *transport);

    // agent functions
    // TODO: MOVE THESE TO THE AGENT INFO CLASS
    void task_agent(int agent_op, char *dat_ptr, char *d_ptr);
    static void register_agent(char *id, char *pass);
};


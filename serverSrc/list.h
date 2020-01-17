#include "common.h"
#include "misc.h"
#include "typedefs.h"

class List
{
private:
    pClientNode first;
    pClientNode last;
    int size;
    pthread_mutex_t session_lock;
public:
    List();
    ~List();
    int remove_node(pClientNode node);
    int add_node(pClientNode node);
    int get_size();
};


void list_add_node(struct clientNode *node, struct clientNode *prevNode);
void list_remove_node(struct clientNode *node);
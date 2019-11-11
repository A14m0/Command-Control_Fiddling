#include <libssh/libssh.h>
#include "misc.h"

struct clientNode {
    struct clientNode *nxt;
    struct clientNode *prev;
    clientDat *data;
};

void list_add_node(struct clientNode *node, struct clientNode *prevNode);
void list_remove_node(struct clientNode *node);
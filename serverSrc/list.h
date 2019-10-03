#include "misc.h"
struct clientNode {
    struct clientNode *nxt;
    struct clientNode *prev;
    clientDat *data;
};

void add_node(struct clientNode *node, struct clientNode *prevNode);
void remove_node(struct clientNode *node);
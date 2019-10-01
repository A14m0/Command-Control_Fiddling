struct clientNode
{
    struct clientNode *nxt;
    struct clientNode *prev;
    struct clientDat *data;
};

void app_node(struct clientNode node, struct clientNode prevNode);
void remove_node(struct clientNode node);
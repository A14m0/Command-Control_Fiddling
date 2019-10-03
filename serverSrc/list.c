#include "list.h"

void remove_node(struct clientNode *node){
    printf("Removing node...\n");
    if(node->nxt != NULL){
        (node->nxt)->prev = node->prev;
    }
    
    node->prev->nxt = node->nxt;
    printf("Removed node\n");
}

void add_node(struct clientNode *node, struct clientNode *prevNode){
    if(prevNode->nxt == 0){
        prevNode->nxt = node;
        node->prev = prevNode;
    } else {
        struct clientNode *tmp = prevNode->nxt;
        prevNode->nxt = node;
        node->prev = prevNode;
        node->nxt = tmp;
        tmp->prev = node;
    }
}

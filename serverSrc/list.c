#include "list.h"

void list_remove_node(struct clientNode *node){
    if(node->nxt != NULL){
        (node->nxt)->prev = node->prev;
    }
    
    node->prev->nxt = node->nxt;
    free(node);
}

void list_add_node(struct clientNode *node, struct clientNode *prevNode){
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

int _list_size(struct clientNode *entry){
    int ctr = 0;
    struct clientNode *curr;
    curr = entry;
    while(curr->nxt != NULL){
        ctr++;
        curr = curr->nxt;
    }
    return ctr;
}
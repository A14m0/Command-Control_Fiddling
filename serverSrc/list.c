#include "list.h"

void remove_node(struct clientNode *node){
    if(node->nxt != NULL){
        (node->nxt)->prev = node->prev;
    }
    
    node->prev->nxt = node->nxt;
    free(node);
}

void add_node(struct clientNode *node, struct clientNode *prevNode){
    if(prevNode->nxt == 0){
        printf("Adding node to end of list\n");
        prevNode->nxt = node;
        node->prev = prevNode;
    } else {
        printf("Adding node in the middle thing\n");
        struct clientNode *tmp = prevNode->nxt;
        prevNode->nxt = node;
        node->prev = prevNode;
        node->nxt = tmp;
        tmp->prev = node;
    }
}

int list_size(struct clientNode *entry){
    int ctr = 0;
    struct clientNode *curr;
    curr = entry;
    while(curr->nxt != NULL){
        ctr++;
        curr = curr->nxt;
    }
    return ctr;
}
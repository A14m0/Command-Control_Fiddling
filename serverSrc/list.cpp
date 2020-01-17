#include "list.h"

List::List(){
    this->first = NULL;
    this->last = NULL;
    this->size = 0;
}

int List::remove_node(pClientNode node){
    pthread_mutex_lock(&(this->session_lock));
    if (!node)
    {
        printf("Node already freed!\n");
        pthread_mutex_unlock(&(this->session_lock));
        this->size--;
        return;
    }
    
    if(node->nxt != NULL){
        (node->nxt)->prev = node->prev;
    }
    
    ssh_channel_close(node->data->chan);
    if(node->data->chan) ssh_channel_free(node->data->chan);
    if (!node->data)
    {
        if (node->data->id)
        {
            free(node->data->id);
        }
    
        free(node->data);
    }
    
    node->prev->nxt = node->nxt;
    free(node);    
    pthread_mutex_unlock(&(this->session_lock));
    this->size--;
}

int List::add_node(pClientNode node){
    pthread_mutex_lock(&(this->session_lock));
    this->last->nxt = node;
    node->prev = this->last;
    node->nxt = nullptr;
    this->last = node;
    pthread_mutex_unlock(&(this->session_lock));
    this->size++;
}

int List::get_size(){
    return this->size;
}
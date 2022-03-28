#include "shell.h"

ShellComms::ShellComms(/* args */)
{
    this->session = (p_avail_sess)malloc(sizeof(avail_sess));
    this->session->session_id = rand() % 1024;
    sprintf(this->session->name, "%s", "None");
    this->session->shared_mem = malloc(4096);
}

ShellComms::~ShellComms()
{
}

int ShellComms::get_id(){
    return this->session->session_id;
}

char *ShellComms::get_name(){
    return this->session->name;
}

int ShellComms::start_session(){
    this->state = ACTIVE_STATE;
    return 0;
}

int ShellComms::wait_read(void** buffer, int len, int flag){
    while(this->state != flag){
        ;
    }

    pthread_mutex_lock(&(this->memlock));
    if(len > 4096) len = 4096;
    memcpy(*buffer, this->session->shared_mem, len);
    memset(this->session->shared_mem, 0, 4096);
    this->state = ACTIVE_STATE;
    pthread_mutex_unlock(&(this->memlock));
    return 0;
}

int ShellComms::wait_write(void *buffer, int len, int flag){
    while(this->state != flag){
        ;
    }

    pthread_mutex_lock(&(this->memlock));
    if(len > 4096) len = 4096;
    memcpy(this->session->shared_mem, buffer, len);
    this->state = flag;
    pthread_mutex_unlock(&(this->memlock));
    return 0;
}
bool ShellComms::check_state(int target_state){
    if (this->state == target_state){
        return true;
    }
    return false;
}
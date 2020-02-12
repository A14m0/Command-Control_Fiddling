#include "log.h"
extern FILE *log_file;

Log::Log(){
    this->logfile = fopen("log.txt", "a");
    if (!this->logfile)
    {
        printf("Log file failed to open!\n");
        perror("Reason");
        exit(1);
    }
    pthread_mutex_init(&(this->session_lock), NULL);
}

int Log::log(char *data){
    pthread_mutex_lock(&(this->session_lock));
    if (!this->logfile)
    {
        printf("Log File not open!!! WTF???\n%s\n", data);
        pthread_mutex_unlock(&(this->session_lock));
        return 1;
    }
    
    fwrite(data, 1, strlen(data), this->logfile);
    printf("%s", data);
    pthread_mutex_unlock(&(this->session_lock));
    return 0;
}

Log::~Log(){
    fclose(this->logfile);
}
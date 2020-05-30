#include "log.h"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

Log::Log(){
    this->logfile = fopen("log.txt", "a");
    if (this->logfile == nullptr)
    {
        printf("Log file failed to open!\n");
        perror("Reason");
        exit(1);
    }
}


int Log::log(char *buff){
    int rc = 0;
    
    if (!this->logfile)
    {
        rc = this->open_log();
        if(rc){
            printf("Log File failed to open!!! WTF???\n%s\n");
            return 1;
        }
    }

    pthread_mutex_lock(&lock);
    
    fwrite(buff, 1, strlen(buff), this->logfile);
    printf("%s", buff);
    pthread_mutex_unlock(&lock);
    this->close_log();
    return 0;
}

int Log::open_log(){
    this->logfile = fopen("log.txt", "a");
    if (this->logfile == nullptr)
    {
        printf("Log file failed to open!\n");
        perror("Reason");
        return 1;
    }
    return 0;
}

int Log::close_log(){
    fclose(this->logfile);
    return 0;
}

Log::~Log(){
    this->close_log();
}
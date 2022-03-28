#include "log.h"

// multithreaded lock for accessing file
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

Log::Log(){
    // open the log file 
    if (this->open_log())
    {
        exit(1);
    }
}


/* Logs data in `buff` to both stdout and the logfile*/
int Log::log(char *buff){
    int rc = 0;
    
    if (!this->logfile)
    {
        rc = this->open_log();
        if(rc){
            printf("Log File failed to open!!! WTF???\n");
            return 1;
        }
    }

    pthread_mutex_lock(&lock);
    
    fwrite(buff, 1, strlen(buff), this->logfile);
    printf("%s", buff);
    pthread_mutex_unlock(&lock);
    
    return 0;
}

/* Opens the log file */
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

/* Close the log file*/
int Log::close_log(){
    fclose(this->logfile);
    return 0;
}

Log::~Log(){
    this->close_log();
}
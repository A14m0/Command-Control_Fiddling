#include "log.h"

Log::Log(){
    this->logfile = fopen("log.txt", "a");
    if (this->logfile == nullptr)
    {
        printf("Log file failed to open!\n");
        perror("Reason");
        exit(1);
    }
    pthread_mutex_init(&(this->session_lock), NULL);
}

int Log::log(const char *frmt_str, char *id, ...){
    pthread_mutex_lock(&(this->session_lock));
    int rc = this->open_log();
    if(!rc){
        return 1;
    }

    char logbuff[2048];
    memset(logbuff, 0, sizeof(logbuff));
    va_list args;
    va_start(args, id);

    snprintf(logbuff, sizeof(logbuff), frmt_str, id, args);
    va_end(args);

    if (!this->logfile)
    {
        printf("Log File not open!!! WTF???\n%s\n", logbuff);
        pthread_mutex_unlock(&(this->session_lock));
        return 1;
    }
    
    fwrite(logbuff, 1, strlen(logbuff), this->logfile);
    printf("%s", logbuff);
    pthread_mutex_unlock(&(this->session_lock));
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
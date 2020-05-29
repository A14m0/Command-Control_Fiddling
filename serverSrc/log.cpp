#include "log.h"

Log::Log(){
    this->logfile = fopen("log.txt", "a");
    if (this->logfile == nullptr)
    {
        printf("Log file failed to open!\n");
        perror("Reason");
        exit(1);
    }
    this->session_lock = *(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(&(this->session_lock), NULL);
    printf("Address of lock: %p\n", &this->session_lock);
}

Log::Log(pthread_mutex_t lock){
    this->logfile = fopen("log.txt", "a");
    if (this->logfile == nullptr)
    {
        printf("Log file failed to open!\n");
        perror("Reason");
        exit(1);
    }
    this->session_lock = lock;
}

int Log::log(const char *frmt_str, char *id, ...){
    int rc = 0;
    
    if (!this->logfile)
    {
        rc = this->open_log();
        if(rc){
            printf("Log File failed to open!!! WTF???\n%s\n");
            pthread_mutex_unlock(&(this->session_lock));
            return 1;
        }
    }

    printf("Logger is waiting for mutex lock\n");
    printf("Mutex address: %p\n", &this->session_lock);
    pthread_mutex_lock(&(this->session_lock));
    
    
    char logbuff[2048];
    memset(logbuff, 0, sizeof(logbuff));
    va_list args;

    va_start(args, id);
    snprintf(logbuff, sizeof(logbuff), frmt_str, id, args);
    va_end(args);

    
    
    fwrite(logbuff, 1, strlen(logbuff), this->logfile);
    printf("%s", logbuff);
    pthread_mutex_unlock(&(this->session_lock));
    printf("Logger released lock\n");
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

pthread_mutex_t Log::get_mutex(){
    return this->session_lock;
}

Log::~Log(){
    this->close_log();
}
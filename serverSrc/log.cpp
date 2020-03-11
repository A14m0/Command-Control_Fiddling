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

int Log::log(char *frmt_str, char *id, ...){
    pthread_mutex_lock(&(this->session_lock));
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
    return 0;
}

Log::~Log(){
    fclose(this->logfile);
}
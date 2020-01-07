#include "log.h"
extern FILE *log_file;

int init_log(){
    log_file = fopen("log.txt", "a");
    if (!log_file)
    {
        return 1;
    }
    return 0;
}

void log_info(char *data){
    if (!log_file)
    {
        printf("Log File not open!!! WTF???\n%s\n", data);
        return;
    }
    
    fwrite(data, 1, strlen(data), log_file);
    printf("%s", data);
}

void close_log(){
    fclose(log_file);
}
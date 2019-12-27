#include "log.h"
FILE *log_file;

int init_log(){
    log_file = fopen("log.txt", "a");
    if (!log_file)
    {
        return 1;
    }
    return 0;
}

void log_info(char *data){
    fwrite(log_file, 1, strlen(data), data);
    printf("%s", data);
}

void close_log(){
    fclose(log_file);
}
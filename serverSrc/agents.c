#include "agents.h"
#include "misc.h"


void init_agent(char *agent_id){
    FILE *fd = NULL;
    char buff[2048];
    memset(buff, 0, sizeof(buff));
    strcat(buff, "agents/");
    strcat(buff, agent_id);
    mkdir(buff, 0666);
    mkdir(strcat(buff, "loot"), 0666);

    fd = fopen(strcat(buff, "agent.mfst"), "w");
    char data[512];

    /*
    prep data in here
    */


    fwrite(data, 1, strlen(data), fd);
    fclose(fd);
}

int get_tasking(char *agent_id, char *tasking){
    strcat(tasking, "NULL :)");
    return 0;
}

int get_file(char *name, char *ptr){
    int size = 0;
    FILE *file;
    file = fopen(name, "r");
    if (file)
    {
        fclose(file);
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    ptr = malloc(size);
    fread(ptr, 1, size, file);
    fclose(file);

    return size;
}


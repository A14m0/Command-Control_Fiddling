#include "agents.h"
#include "misc.h"


void init_agent(char *agent_id){
    FILE *fd = NULL;
    char buff[2048];
    char buff2[2048];
    memset(buff, 0, sizeof(buff));
    memset(buff2, 0, sizeof(buff2));
    strcat(buff, "agents/");
    strcpy(buff2, buff);
    strcat(buff, agent_id);
    mkdir(buff, 0111);
    mkdir(strcat(buff, "-loot"), 0111);

    fd = fopen("agent.mfst", "w");
    char data[512];

    fwrite("default", 1, sizeof("default"), fd);


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


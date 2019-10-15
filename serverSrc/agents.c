#include "agents.h"
#include "misc.h"

void init_agent(char *agent_id){
    FILE *fd = NULL;
    char buff[2048];
    char buff2[2048];
    memset(buff, 0, sizeof(buff));
    memset(buff2, 0, sizeof(buff2));
    strcat(buff, "agents/");
    strcat(buff, agent_id); //agents/TEST-AGENT
    strcpy(buff2, buff);
    mkdir(buff, 0755);
    mkdir(strcat(buff, "/loot"), 0755);

    fd = fopen(strcat(buff2, "/agent.mfst"), "w");
    char data[512];

    fwrite("default", 1, sizeof("default"), fd);


    fwrite(data, 1, strlen(data), fd);
    fclose(fd);
}

void write_format(char *path){
    FILE *fd;
    fd = fopen(path, "w");
    fwrite("NULL :)\n", 1, sizeof("NULL :)"), fd);
    fclose(fd);
}

int get_tasking(char *agent_id, char *tasking){
    char file[2048];

    memset(file, 0, sizeof(file));
    sprintf(file, "agents/%s/agent.mfst", agent_id);
    FILE *fd = NULL;
    fd = fopen(file, "r");
    fseek(fd, 0L, SEEK_END);
    int size = ftell(fd);
    rewind(fd);
    char *mem_dump = malloc(size);
    memset(mem_dump, 0, size);
    fread(mem_dump, 1, size, fd);
    fclose(fd);

    strcat(tasking, mem_dump);
    //write_format(file);
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

void compile_agent(char *ip, char *port){
    // In here the agent file header is editeed and recompiled against these values
    // Also in here is where the username and password are added to the server database
    printf("Not working yet. Working on it ;)\n");

    // move over the required source files
    char *buff[BUFSIZ];
    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "client.c");
    copy_file(buff, "out/client.c");

    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.h");
    copy_file(buff, "out/agent.h");

    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.c");
    copy_file(buff, "out/agent.c");

    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "examples_common.h");
    copy_file(buff, "out/examples_common.h");

    // create config file
    FILE *fd = NULL;
    fd = fopen("out/config.h", "w");

    memset(buff, 0, sizeof(buff));
    strcat(buff, "#define IP ");
    strcat(buff, ip);
    strcat(buff, "\n#define PORT ");
    strcat(buff, port);
    strcat(buff, "\n");
    fwrite(buff, 1, strlen(buff) -1, fd);

    printf("IP: %s\n", ip);
    printf("Port: %s\n", port);

    printf("Compiling agent...\n");
    system(COMPILE);
}

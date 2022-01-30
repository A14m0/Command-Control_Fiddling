#include "ssh_transport.h"
#include <signal.h>

// server required defines
int type = TRANSPORT; // type TRANSPORT
char *t_name = "SSH Backend";
int t_id = 55;



// instance data
typedef struct _dat_str {
    int portno;
    char *agent_name;
    ssh_bind sshbind;
    ssh_session session;
    ssh_channel channel;
} data_struct, *pdata_struct;


// API struct resolved at runtime, points to all the required functions
extern "C"{
    void *generate_class(){
        SshTransport *ptr = new SshTransport();
        return ptr;
    }
}









// initializes the instance data
SshTransport::SshTransport() {
    id = t_id;
    name = t_name;
    agent_name = (char*)malloc(128);
    if(agent_name == nullptr) {
        printf("SSHTRANSPORT NULLPTR !!!!!!!!!!!!!!!!!!!!!\n");
    }
}

// frees instance data and variables
SshTransport::~SshTransport() {
    free(agent_name);
    ssh_bind_free(sshbind);
    ssh_finalize();
}

// PRIVATE: checked write to channel
int SshTransport::write(char *buffer, unsigned long len) {
	// writes data to channel
    int rc = ssh_channel_write(this->channel, buffer, len);
    if(rc == SSH_ERROR){
        printf("Failed to write data to channel: %s\n", ssh_get_error(ssh_channel_get_session(this->channel)));
    }
	return rc;
}

// PRIVATE: checked read from channel
//int SshTransport::read(char** buffer, int len) {
unsigned char *SshTransport::read(unsigned long len) {
    //raise(SIGINT);

    // make sure we arent allocating bogus memory sizes
    if(len < 1) {
        printf("TRANSPORT: Caught illicit read length: %lu\n", len);
        return nullptr;
    }
    printf("Received length: %lu\n", len);

    unsigned char *b = (unsigned char*)malloc(len);
	// sanity check buffer
	if(b == nullptr) {
        printf("B ADDR: %p\n", b);
        perror("");
        exit(1);
	} 

    if(len > UINT32_MAX ) {
        // if we get a number thats bigger than `ssh_channel_read` can handle, 
        // we do a special looping read until we are done
        
        unsigned long ctr = 0;
        while(ctr < len){
            // read data
            int readlen = 0;
            if(len > UINT32_MAX){
                readlen = UINT32_MAX;
            } else {
                readlen = len-ctr;
            }

	        int nbytes = ssh_channel_read(this->channel, b, readlen, 0);
	        if (nbytes < 0){
	        	printf("Caught read error from server: %s\n", ssh_get_error(ssh_channel_get_session(this->channel)));
            	free(b);
                return nullptr;
	        }
            ctr += nbytes;
        }
            
	
    } else {
        // read data
	    int nbytes = ssh_channel_read(this->channel, b, len, 0);
	    printf("read %d bytes from channel\n", nbytes);
	    if (nbytes < 0){
	    	printf("Caught read error from server: %s\n", ssh_get_error(ssh_channel_get_session(this->channel)));
        	return nullptr;
	    }
	
    }
	
	// all good
	return b;
}


// fetches tasking from the remote agent
api_return SshTransport::fetch_tasking() {
    // read the header
    char header[8] = {0};
    //int rc = this->read((char**)&header, 8); 
    printf("\n-------------------------------------------------\n");
    printf("TSPT: READING...\n");
	unsigned char *hdr = this->read(8);
    printf("hdr: "); 
    for(int i = 0; i < 8; i++) {
        printf("%x ", hdr[i]);
    }
    printf("\n"); 
    
    if(hdr == nullptr) return api_return{API_ERR_READ, nullptr};
	unsigned long h_val = AgentJob::bytes_to_long(hdr);
    free(hdr);
	AgentJob *job = new AgentJob(h_val, nullptr);
    printf("Tasking values: %x, %lu\n", job->get_type(), job->get_len());
    printf("-------------------------------------------------\n\n");

	// now that we know the length, fetch the payload
	unsigned char *data = this->read(job->get_len()); 
	if(data == nullptr) {
		// free stuff if the read fails
		delete job;
		return api_return{API_ERR_READ, nullptr};
	}

	// set payload and return
	job->set_data(data);
    return api_return{API_OK, job};
}

// pushes tasking to the remote agent
api_return SshTransport::push_tasking(ptask_t task){
    // first we pack the task
    AgentJob *job = new AgentJob(task);
    int packed_size = job->get_len()+8;
    void *packed = job->pack();

    // now we send it
    if(this->write((char*)packed, packed_size) == SSH_ERROR) return api_return{API_ERR_WRITE, (void*)ssh_get_error(ssh_channel_get_session(this->channel))};

    return api_return{API_OK, nullptr};
}

// listens for an incoming connection
api_return SshTransport::listen() {
    int r;

    int sock = socket(AF_INET , SOCK_STREAM , 0);

    sshbind=ssh_bind_new();
    session=ssh_new();

    ssh_options_set(session, SSH_OPTIONS_FD, &sock);

    //ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");
    //printf("Binding to portno %d...\n", tmp->portno);
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &(portno));

    if(ssh_bind_listen(sshbind)<0){
        return api_return{API_ERR_LISTEN, (void*) ssh_get_error(sshbind)};
    }

    // bind the listener to the port
    r=ssh_bind_accept(sshbind, session);
    if(r==SSH_ERROR){
      	return api_return{API_ERR_ACCEPT, (void*) ssh_get_error(sshbind)};
    }
    if (ssh_handle_key_exchange(session)) {
        return api_return{API_ERR_AUTH, (void*) ssh_get_error(sshbind)};
    }

    if (!Authenticate()) {
        return api_return{API_ERR_AUTH, (void*) "Bad authentication"};
    }

    return api_return{API_OK, nullptr};
}

// returns the transport's name
const char *SshTransport::get_tname(){
    return name;
}

// returns the currently connected agent's name
api_return SshTransport::get_aname() {
    return api_return{API_OK, this->agent_name};
}

// returns the transport's ID
int SshTransport::get_id(){
    return id;
}

// sets the port to listen on
api_return SshTransport::set_port(int portno){
    portno = portno;

    return api_return{API_OK, nullptr};
}


/////////////////////////// HELPER FUNCTIONS ///////////////////////////////////

int SshTransport::Authenticate(){
    // initialize variables
    int auth=0;
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
                        {
                            // generate tasking info
                            pauth_t auth_struct = (pauth_t)malloc(sizeof(auth_t));
                            strncpy(auth_struct->uname, ssh_message_auth_user(message), 128);
                            strncpy(auth_struct->passwd, ssh_message_auth_password(message), 64);

                            // send tasking to server and await tasking report
                            if(Common::authenticate(auth_struct) == RESP_OK) {
                                auth=1;

                                memset(agent_name, 0, 128);//strlen(ssh_message_auth_user(message)));
                                snprintf(agent_name, 128, "%s", ssh_message_auth_user(message));
                                ssh_message_auth_reply_success(message,0);
                                break;
                            } else {
                                auth = 2;
                                ssh_message_reply_default(message);
                                break;
                            }
                        }
                        // not authenticated, send default message
                    case SSH_AUTH_METHOD_NONE:
                        printf("lol wtf (Line 168 ssh_transport)\n");

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
        ssh_disconnect(session);
        return 0;
    } else {
        this->DetermineHandler();
        return 1;
    }
    return 0;
}

int SshTransport::DetermineHandler(){
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
        ssh_finalize();
        return 0;
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
        
        if(tmp_buffer[0] == '9'){
            //ssh_channel_write(channel, "ok", 2);
            return MANAG_TYPE;

        } else {
            // Send OK
            //rc = ssh_channel_write(channel, "ok", 3);
            if(rc == SSH_ERROR){
                printf("Client %s: caught ssh error: %s", agent_name, ssh_get_error(session));
                api_return{API_ERR_GENERIC, (void*)ssh_get_error(session)};
            }
            
            /* NOTE: THIS NEEDS TO MOVE */
////////////////////////////////////////////////////////////////////////////////
            //memset(buf, 0, sizeof(buf));
            //strcat(buf, "agents/");
            //int exists = Common::directory_exists(strcat(buf, agent_name));
//
            //if(!exists){
            //    Common::init_agent(agent_name);
            //    printf("Client %s: Initialized agent\n", agent_name);
            //}
////////////////////////////////////////////////////////////////////////////////

            // Pass to handler
            return AGENT_TYPE;

        }

        break;
    default:
        break;
        //p_ref->log(LOG_ERROR, "Client %s: got unknown message type: %d\n", agent_name, msgType);
    }

    // p_ref->log(LOG_INFO, "Transport: Closing channels...\n");
    ssh_message_free(message);
    ssh_finalize();



    return 0;

}

/*
api_return determine_handler(void* instance_struct){
    data_struct *dat_structure = (data_struct*)instance_struct;

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
        return api_return{API_ERR_GENERIC, (void *)ssh_get_error(session)};
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

        if(tmp_buffer[0] == '9'){
            ssh_channel_write(channel, "ok", 2);
            return api_return{API_OK, (void *)MANAG_TYPE};

        } else {
            // Check if ID exists
            memset(buf, 0, sizeof(buf));
            strcat(buf, "agents/");
            int exists = misc_directory_exists(strcat(buf, agent_name));

            if(!exists){
                AgentInformationHandler::init(agent_name);
                printf("Client %s: Initialized agent\n", agent_name);
            }
            rc = ssh_channel_write(channel, "ok", 3);
            if(rc == SSH_ERROR){
                printf("Client %s: caught ssh error: %s", agent_name, ssh_get_error(session));
                api_return{API_ERR_GENERIC, (void*)ssh_get_error(session)};
            }

            rc = ssh_channel_read(channel, beacon, sizeof(beacon), 0);
            if(rc == SSH_ERROR){
                printf("Client %s: caught ssh error: %s", agent_name, ssh_get_error(session));
                api_return{API_ERR_GENERIC, (void *) ssh_get_error(session)};
            }
            AgentInformationHandler::write_beacon(agent_name, beacon);

            tasking = AgentInformationHandler::get_tasking(agent_name);
            if(!tasking){
                printf("Client %s: caught ssh error: %s", agent_name, ssh_get_error(session));
                perror("Reason");
                api_return{API_ERR_GENERIC, (void *) ssh_get_error(session)};
            }

            // Write tasking
            rc = ssh_channel_write(channel, tasking, strlen(tasking));
            if(rc == SSH_ERROR){
                printf("Client %s: Failed to write to channel: %s", agent_name, ssh_get_error(session));
                api_return{API_ERR_READ, (void *) ssh_get_error(session)};

            }
            // Pass to handler
            return api_return{API_OK, (void*) AGENT_TYPE};

        }

        break;
    default:
        printf("Client %s: got unknown message type: %d\n", agent_name, msgType);
        api_return{API_ERR_CLIENT, (void*)ssh_get_error(session)};
    }

    printf("Closing channels...\n");
    ssh_message_free(message);
    ssh_finalize();

    return api_return{API_ERR_GENERIC, nullptr};
}

/*Uploads a file to some connected entity
api_return upload_file(void* instance_struct, const char *ptr, int is_module){
    data_struct *dat_structure = (data_struct*)instance_struct;


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
    printf("Client %s: Sending file -> %s\n", agent_name, ptr);

    // get filesize
    snprintf(buff,8000, "%s/%s", getcwd(directory, sizeof(directory)), ptr);
    size = misc_get_file(buff, &file_data);

    if(size < 0){
        printf("Client %s: filename '%s' does not exist\n", agent_name, buff);

        rc = ssh_channel_write(channel, "er", 3);
        if(rc == SSH_ERROR){
            printf("Client %s: Failed to write data to channel: %s\n", agent_name, ssh_get_error(session));
            return api_return{API_ERR_WRITE, (void*) ssh_get_error(session)};
        }
        return api_return{API_ERR_LOCAL, (void*)"File not found"};
    }

    size_e = B64::enc_size(size);
    sprintf(tmpbuffer, "%d", size_e);

    // writes file size
    rc = ssh_channel_write(channel, tmpbuffer, sizeof(tmpbuffer));
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to write data to channel: %s\n", agent_name, ssh_get_error(session));
        return api_return{API_ERR_WRITE, (void*) ssh_get_error(session)};
    }

    rc = ssh_channel_read(channel, buff, sizeof(buff), 0);
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to read data from channel: %s\n", agent_name, ssh_get_error(session));
        return api_return{API_ERR_READ, (void*) ssh_get_error(session)};
    }

     B64::encode((unsigned char *)file_data, size, &enc_data);

    // writes file
    rc = ssh_channel_write(channel, enc_data, size_e);
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to write data to channel: %s\n", agent_name, ssh_get_error(session));
        return api_return{API_ERR_WRITE, (void*) ssh_get_error(session)};
    }
    memset(tmpbuffer, 0, 8);

    rc = ssh_channel_read(channel, tmpbuffer, 8, 0);
    if(rc == SSH_ERROR){
        printf("Client %s: Failed to read data from channel: %s\n", agent_name, ssh_get_error(session));
        return api_return{API_ERR_READ, (void*) ssh_get_error(session)};
    }


    if(is_module){
        printf("Client %s: Execution of module ended with exit code %s\n", agent_name, tmpbuffer);
    }
    free(file_data);
    free(enc_data);

    return api_return{API_OK, nullptr};
}

api_return init_reverse_shell(void* instance_struct){
    data_struct dat_structure = *(data_struct*)instance_struct;

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
    }#0  Server::handle_instance (server=0x555555580730, handle=0x555555589c90, reload=true) at serverSrc/server.cpp:209

    inst->shell_finish();
    return api_return{API_ERR_WRITE, nullptr};
}

api_return listen(void* instance_struct){
    data_struct *dat_structure = (data_struct*)instance_struct;

    printf("[Listen] Address %p->%p\n", instance_struct, dat_structure);


    int r;

    int sock = socket(AF_INET , SOCK_STREAM , 0);

    sshbind=ssh_bind_new();
    session=ssh_new();

    ssh_options_set(session, SSH_OPTIONS_FD, &sock);

    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");
    printf("Binding to portno %d...\n", portno);
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &(portno));

    if(ssh_bind_listen(sshbind)<0){
        printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
        return api_return{API_ERR_LISTEN, (void*) ssh_get_error(sshbind)};
    }

    // bind the listener to the port
    printf("Server: Bound to listening port\n");

    r=ssh_bind_accept(sshbind, session);
    printf("Server: Accepting connection\n");
    if(r==SSH_ERROR){
      	printf("Error accepting a connection : %s\n",ssh_get_error(sshbind));
        return api_return{API_ERR_ACCEPT, (void*) ssh_get_error(sshbind)};
    }
    if (ssh_handle_key_exchange(session)) {
        printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
        return api_return{API_ERR_AUTH, (void*) ssh_get_error(sshbind)};
    }

    api_return rc = authenticate(&instance_struct);
    if (rc.error_code != API_OK)
    {
        printf("Initialization: Failed to authenticate agent\n");
        return api_return{API_ERR_AUTH, (void*) "Bad authentication"};
    }
    return api_return{API_OK, nullptr};

}


api_return authenticate(void** instance_struct){
    data_struct *dat_structure = *(data_struct**)instance_struct;

    // initialize variables
    int auth=0;
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

                            // TODO: FIX TF OUT OF THIS
                            //name = (char*)malloc(strlen(ssh_message_auth_user(message)));

                            if(agent_name == NULL){
                                printf("all the way down...\n");
                            }
                            memset(agent_name, 0, sizeof(agent_name));//strlen(ssh_message_auth_user(message)));
                            snprintf(agent_name, sizeof(agent_name), "%s", ssh_message_auth_user(message));
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
        return api_return{API_OK, (void*) 1};
    } else {
        return api_return{API_OK, (void*) 0};
    }
    return api_return{API_ERR_GENERIC, (void*) "Unreachable"};
}

api_return read(void* instance_struct, char **buff, int length){
    data_struct *dat_structure = (data_struct*)instance_struct;

    int rc = 0;
    rc = ssh_channel_read(channel, *buff, length, 0);
    if (rc == SSH_ERROR)
    {
        printf("Failed to handle agent: %s\n", ssh_get_error(session));
        return api_return{API_ERR_READ, (void*) ssh_get_error(session)};
    }
    return api_return{API_OK, (void*) rc};

}

api_return write(void* instance_struct, const char *buff, int length){
    data_struct *dat_structure = (data_struct*)instance_struct;

    int rc = 0;
    rc = ssh_channel_write(channel, buff, length);
    if(rc == SSH_ERROR){
        printf("Failed to handle agent: %s\n", ssh_get_error(session));
        return api_return{API_ERR_WRITE, (void*) ssh_get_error(session)};
    }
    return api_return{API_OK, (void*) rc};
}

api_return send_err(void* instance_struct){
    data_struct *dat_structure = (data_struct*)instance_struct;

    int rc = 0;
    rc = ssh_channel_write(channel, "er", 3);
    if (rc == SSH_ERROR)
    {
        printf("Failed to handle agent: %s\n", ssh_get_error(session));
        return api_return{API_ERR_WRITE, (void*) ssh_get_error(session)};
    }
    return api_return{API_OK, nullptr};
}

api_return send_ok(void* instance_struct){
    data_struct *dat_structure = (data_struct*)instance_struct;

    int rc = 0;
    rc = ssh_channel_write(channel, "ok", 3);
    if (rc == SSH_ERROR)
    {
        printf("Failed to handle agent: %s\n", ssh_get_error(session));
        return api_return{API_ERR_WRITE, (void*) ssh_get_error(session)};
    }
    return api_return{API_OK, nullptr};
}

api_return set_port(void* instance_struct, int portno){
    data_struct *dat_structure = (data_struct*)instance_struct;
    printf("Address %p->%p\n", instance_struct, dat_structure);

    fflush(stdout);

    printf("received port number %d\n", portno);
    if(portno == 0){
        printf("Portno was zero, like it should be...\n");
        portno = default_port;
    } else {
        portno = portno;
    }

    printf("Current port setting: %d\n", portno);

    return api_return{API_OK, nullptr};
}

api_return get_agent_name(void* instance_struct){
    api_return ret;
    data_struct *dat_structure = (data_struct*)instance_struct;

    return api_return{API_OK, agent_name};
}*/

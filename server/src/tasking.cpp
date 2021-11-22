#include "tasking.h"
#include <stdio.h>
#include <stdlib.h>

// constructor for our AgentJob class
AgentJob::AgentJob(unsigned char type, unsigned long len, void* data) {
    // sanity check the len to make sure it isn't too big
    if( (len & 0xff00000000000000) != 0) {
        printf("Len too long!");
        exit(1);
    }
    this->type = type;
    this->len = len;
    this->data = data;
}

AgentJob::AgentJob(unsigned long combined, void* data) {
    // Type: (char) ((combined & 0xff00000000000000) >>7)
    unsigned char t = (unsigned char) ((combined & 0xff00000000000000) >> 56);
    unsigned long l = (unsigned long) (combined & 0x00ffffffffffffff);
    this->type = t;
    this->len = l;
    this->data = data;
}


// helper function to get the type of an agent job strucutre
unsigned char AgentJob::get_type() {
    return this->type;
}

// helper function to get the length of an agent job's data
unsigned long AgentJob::get_len() {
    return this->len;
}

// helper function to get the raw data of an agent job
void *AgentJob::get_data() {
    return this->data;  
}

// helper function that generates a header long for the agent job
unsigned long AgentJob::encode_header(){
    unsigned long t = (unsigned long)(this->type);
    t = t << 56;
    return t | this->len;
}
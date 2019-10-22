#pragma once
#include "agents.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>

typedef struct {
    int id;
    ssh_session session;
    int trans_id;
    ssh_channel chan;
    int type;
}clientDat;

int index_of(char* str, char find, int rev);
int directory_exists( const char* pzPath );
void clean_input(char *input);
void init();
int copy_file(char *filename, char *dest);
char *substring(char *string, int position, int length);
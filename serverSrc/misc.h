#pragma once
#include "agents.h"
#include <signal.h>

typedef struct {
    int id;
    ssh_session session;
    int trans_id;
    ssh_channel chan;
    int type;
}clientDat;

int auth_password(const char *user, const char *password);
int index_of(char* str, char find, int rev);
int directory_exists( const char* pzPath );
void clean_input(char *input);
void init();
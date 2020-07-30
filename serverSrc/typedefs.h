#pragma once

//#include "common.h"

#define MODULE 69
#define TRANSPORT 99

typedef struct clientDat{
    char id[256];
    int type;
    pthread_t thread;
} ClientDat, *pClientDat;

typedef struct _clientNode {
    _clientNode *nxt;
    _clientNode *prev;
    pClientDat data;
} ClientNode, *pClientNode;


typedef struct ret
{
    char *usr;
    char *passwd;
} Passwd, *pPasswd;



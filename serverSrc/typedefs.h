#pragma once

#include "common.h"

typedef struct clientDat{
    char *id;
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



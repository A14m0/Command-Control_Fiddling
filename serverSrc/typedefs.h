#pragma once

#include "common.h"

typedef struct clientDat{
    char *id;
    ssh_session session;
    int trans_id;
    ssh_channel chan;
    int type;
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



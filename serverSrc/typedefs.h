#include "common.h"

typedef struct _clientNode {
    pClientNode nxt;
    pClientNode prev;
    ClientDat *data;
} ClientNode, *pClientNode;


typedef struct ret
{
    char *usr;
    char *passwd;
} Passwd, *pPasswd;


typedef struct clientDat{
    char *id;
    ssh_session session;
    int trans_id;
    ssh_channel chan;
    int type;
} ClientDat, *pClientDat;
#ifndef MSG_H
#define MSG_H

typedef struct
{
    int pid;
    char allomanynev[100];
} Msg;

typedef struct
{
    char katalogusok[100];
} Client;

char* qname = "/server_";



#endif

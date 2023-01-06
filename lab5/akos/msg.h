#ifndef MSG_H
#define MSG_H

typedef struct
{
    int pid;
    char command[256];
} Msg;

typedef struct
{
    char line[256];
} Answer;

char* qname = "/server_";

#endif

//Kovacs Elek Akos
//keim2152
//522/2
//lab5/3

#include "msg_akos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>

void clean(int server_descriptor, int client_descriptor, char* client_query_name, int pid)
{
    if (server_descriptor != -1)
    {
        if (mq_close(server_descriptor) < 0)
        {
            fprintf(stderr, "[%d] - Client - Server Message Query Closing Error!\n", pid);
        }
        else
        {
            fprintf(stderr, "[%d] - Client - Server Message Query Closed Succesfully!\n", pid);
        }
    }

    if (client_descriptor != -1)
    {
        if (mq_close(client_descriptor) < 0)
        {
            fprintf(stderr, "[%d] - Client - My Message Query Closing Error!\n", pid);
        }
        else
        {
            fprintf(stderr, "[%d] - Client - My Message Query Closed Succesfully!\n", pid);
        }
    }

    if (strcmp(client_query_name, ""))
    {
        if (mq_unlink(client_query_name) < 0)
        {
            fprintf(stderr, "[%d] - Client - My Message Query Delete Error!\n", pid);
        }
        else
        {
            fprintf(stderr, "[%d] - Client - My Message Query Deleted Succesfully!\n", pid);
        }
    }
}

int main(int argc, char** argv)
{
    int pid = getpid();

    // Parameter ellenorzes
    if (argc != 2)
    {
        fprintf(stderr, "[%d] - Client - Hibas bemenet!\n", pid);
        exit(1);
    }

    //Szerver uzenetesoranak a megnyitasa
    char serverName[20];
    sprintf(serverName, "%s%s", qname, getlogin());
    mqd_t qid = mq_open(serverName, O_WRONLY);
    if (qid == -1)
    {
        fprintf(stderr, "[%d] - Client - Nem sikerult a server uzenetsort megnyitni!\n", pid);
        exit(1);
    }

    //Kliens uzenetsoranak a megnyitasa
    char clientName[15];
    sprintf(clientName, "/kliens_%d", pid);

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Answer);
    mqd_t cqid = mq_open(clientName, O_RDONLY | O_CREAT, 0600, &attr);

    if (cqid == -1)
    {
        fprintf(stderr, "[%d] - Client - Nem sikerult megnyitni az uzenetsort!\n", pid);
        exit(1);
    }    

    //Keres elkuldese
    Msg msg_client;
    strcpy(msg_client.command, argv[1]);
    msg_client.pid = pid;
    fprintf(stdout, "[%d] - Client - Parancs Elkuldve: %s\n", msg_client.pid, msg_client.command);
    
    if (mq_send(qid, (char*)&msg_client, sizeof(msg_client), 0) == -1)
    {
        fprintf(stderr, "[%d] - Client - Nme sikerult elkuldeni az uzenetet\n", pid);
        clean(qid, cqid, clientName, pid);
        exit(1);
    }

    if(strcmp(msg_client.command,"STOP") == 0){
        clean(qid, cqid, clientName, pid);
        exit(0);
    }

    // Varakozas a valaszra
    fprintf(stdout, "[%d] - Client - Varok a valaszra!\n", pid);
    Answer answer;
    while (1)
    {
        if (mq_receive(cqid, (char*)&answer, sizeof(Answer), 0) < 0)
        {
            fprintf(stdout, "[%d] - Client - A szerver bezarva!\n", pid);
            break;
        }

        if (!strcmp(answer.line, "END_OF_ANSWER"))
        {
            break;
        }
        fprintf(stdout, "%s\n",answer.line);
    }

    clean(qid, cqid, clientName, pid);
    exit(0);
}
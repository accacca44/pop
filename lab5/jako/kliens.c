//Nev: Kovacs Elek Akos
//Azonosito: keim2152
//Csoport: 522/2
//Labor: lab5/3

#include "msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>

void cleaning(int server_descriptor, int client_descriptor, char* client_query_name, int pid);

int main(int argc, char** argv)
{
    /** Parameter ellenorzes **/
    int pid = getpid();

    if (argc != 2)
    {
        fprintf(stderr, "[%d] - Client - Wrong input parameters!\n", pid);
        exit(1);
    }

    /** Szerver uzenetesoranak a megnyitasa **/
    mode_t mode = 0600;
    char sqname[15];
    sprintf(sqname, "%s%s", qname, getlogin());
    mqd_t qid = mq_open(sqname, O_WRONLY);

    if (qid == -1)
    {
        fprintf(stderr, "[%d] - Client - Mq_open error! - Server_Query!\n", pid);
        exit(1);
    }

    /** Kliens uzenetsoranak a megnyitasa **/
    /** A kliens uzenetsora egy masik tipusu strukturat var valaszkent, amiben egyesevel fogja kapni a Directory neveket **/ 
    char cqname[15];

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Client);
    sprintf(cqname, "/kliens_%d", pid);
    mqd_t cqid = mq_open(cqname, O_RDONLY | O_CREAT, mode, &attr);

    if (cqid == -1)
    {
        fprintf(stderr, "[%d] - Client - Mq_open error! - Client_Query!\n", pid);

        exit(1);
    }    

    /** Az elkuldendo uzenet osszerakasa es kiiratasa **/
    Msg msg_client;
    strcpy(msg_client.allomanynev, argv[1]);
    msg_client.pid = pid;

    fprintf(stdout, "[%d] - Client - Sent this filename: %s\n", msg_client.pid, msg_client.allomanynev);
    
    /** Az uzenet elkuldese a szerver uzenetsoraba **/
    if (mq_send(qid, (char*)&msg_client, sizeof(msg_client), 0) == -1)
    {
        fprintf(stderr, "[%d] - Client - Mq_send Error -> To Server Query!\n", pid);
        cleaning(qid, cqid, cqname, pid);
        exit(1);
    }

    /** Az valaszok varasa **/
    fprintf(stdout, "[%d] - Client - I am waiting for answers!\n", pid);
    Client answer;
    while (1)
    {
        if (mq_receive(cqid, (char*)&answer, sizeof(Client), 0) < 0)
        {
            fprintf(stdout, "[%d] - Client - The server is closed!\n", pid);
            break;
        }

        if (!strcmp(answer.katalogusok, "Vege"))
        {
            break;
        }

        if (!strcmp(answer.katalogusok, "0"))
        {
            fprintf(stdout, "[%d] - Client ---> The file \"%s\" is not found in any directoires!\n", pid, argv[1]);
            break;
        }
        fprintf(stdout, "\'%s\' ---> %s\n", argv[1], answer.katalogusok);
    }

    cleaning(qid, cqid, cqname, pid);
    
    exit(0);
}

void cleaning(int server_descriptor, int client_descriptor, char* client_query_name, int pid)
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
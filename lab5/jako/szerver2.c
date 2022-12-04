//Nev: Jako Daniel
//Azonosito: jdim2141
//Csoport: 522/1
//Labor: lab5

//Szerver programkodja

#include "msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAXCLIENT 10

typedef struct data{
    Msg msg;
    int id; 
}data;

void* do_it_server(void* arg);
void cleaning(int server_descriptor, int client_descriptor, char* client_query_name, int pid);


int main(int argx, char** argv)
{
    /** A szerver uzenetsoranak a megnyitasa **/
    mode_t mode = 0600;
    char servername[25];
    sprintf(servername, "%s%s", qname, getlogin());
    struct mq_attr attr; 
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Msg);
    
    mqd_t qid = mq_open(servername, O_RDONLY | O_CREAT, mode, &attr);
    
    if (qid < 0)
    {
        fprintf(stderr, "[Server] - Server Query Open Error!\n");
        exit(1);
    }
    fprintf(stdout, "[Server] - The Server Query is Opened!\n");
    /** Szalak es hozzajuk tartozo strukturak felepitese **/
    pthread_t* th = (pthread_t*)malloc(MAXCLIENT * sizeof(pthread_t));
    data* data_client = (data*)malloc(MAXCLIENT * sizeof(data));

    
    Msg msg_client; 
    int num_client = 0;

    /** Csak adott szamu klienst szolgal ki a szerver **/
    /** Ezt kovetoen leall**/

    while(num_client < MAXCLIENT)
    {
        /** Minden kapott uzenet eseten letrehoz egy szalat az adott kliens kezelesre **/
        if (mq_receive(qid, (char*) &msg_client, sizeof(msg_client), 0) != -1)
        {
            fprintf(stdout, "[Server] - I receive a file ->%s<- from [%d] - Client -\n",msg_client.allomanynev, msg_client.pid);
            
            if(!strcmp(msg_client.allomanynev, "STOP"))
            {
                fprintf(stdout, "[Server] - Server is stopping!\n");
                break;
            }

            data_client[num_client].msg = msg_client;
            data_client[num_client].id = num_client;
            pthread_create(&th[num_client], NULL, do_it_server, &data_client[num_client]);
            num_client++;
        }
        else
        {
            cleaning(qid, -1, "", getpid());
            exit(1);
        }
    }
    
    for (int i = 0; i < num_client; i++)
    {
        pthread_join(th[i], NULL);       
    }

    cleaning(qid, -1, servername, 0);

    free(data_client);
    free(th);
    exit(0);
}

void* do_it_server(void* arg){    
    data* adat = (data*)arg;
    char shell[200];
    sprintf(shell, "./shell.sh %s", adat->msg.allomanynev);

    fprintf(stdout, "[%d] - Thread I manage the ->[%d]<- Client question!\n", adat->id, adat->msg.pid);
    
    /** A Kliens keresenek feldolgozasa popen segitsegevel **/
    /** Keressuk az osszes olyan mappat amelyben megtalalhato az adott allomany **/
    FILE* ps;
    if ((ps = popen(shell, "r")) == NULL)
    {
        fprintf(stderr, "[%d] - Thread Popen Error!\n", adat->id);
        exit(1);
    }

    char szoveg[100];
    char cname[15];
    sprintf(cname, "/kliens_%d", adat->msg.pid);
    Client answer;
    mqd_t cmqid = mq_open(cname, O_WRONLY);  

    fprintf(stdout, "[%d] - Thread sending answers to ->[%d]<- Client!\n", adat->id, adat->msg.pid);
    while(fgets(szoveg,100, ps) != NULL)
    {   
        /** Az fgets bele veszi a sorveget is ezt levalasztjuk manualisan **/
        szoveg[strlen(szoveg) - 1] = '\0';
        strcpy(answer.katalogusok, szoveg);

        /** A popen kimenetet soronkent kuldjuk el a Kliensnek **/
        mq_send(cmqid, (char*)&answer, sizeof(Client), 0);
    }

    /** Az utolso elkuldott uzenet az a Vege szo, hogy a kliens tujda, hogy mikor alljon le**/
    strcpy(answer.katalogusok, "Vege");
    mq_send(cmqid, (char*)&answer, sizeof(Client), 0);
    
    fprintf(stdout, "[%d] - Thread ended the prcess! Cleaning and Closing...\n", adat->id);
    cleaning(-1, cmqid, "", adat->msg.pid);
    return (void*) 0;
}

void cleaning(int server_descriptor, int client_descriptor, char* server_query_name, int pid)
{
    if (server_descriptor != -1)
    {
        if (mq_close(server_descriptor) < 0)
        {
            fprintf(stderr, "[Server] - My Message Query Closing Error!\n");
        }
        else
        {
            fprintf(stderr, "[Server] - My Message Query Closed Succesfully!\n");
        }
    }

    if (client_descriptor != -1)
    {
        if (mq_close(client_descriptor) < 0)
        {
            fprintf(stderr, "[Server] - The [%d] - Client - Query Closing Error!\n", pid);
        }
        else
        {
            fprintf(stderr, "[Server] - The [%d] - Client - Query Closed Succesfully!\n", pid);
        }
    }

    if (strcmp(server_query_name, ""))
    {
        if (mq_unlink(server_query_name) < 0)
        {
            fprintf(stderr, "[Server] - My Message Query Delete Error!\n");
        }
        else
        {
            fprintf(stderr, "[Server] - My Message Query Deleted Succesfully!\n");
        }
    }
}
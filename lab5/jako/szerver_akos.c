//Kovacs Elek Akos
//keim2152
//522/2
//3. Feladat

//Szerver

#include "msg_akos.h"
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

void*routine(void * arg){
    data* myData = (data*)arg;
    
    fprintf(stdout, "[%d] - Thread, feldolgozom a [%d]. kliens kereset!\n", myData->id, myData->msg.pid);

    //popenel lefuttatom a linux parancsokat, majd a valaszokat visszakuldom
    FILE *ansFile;
    if((ansFile = popen(myData->msg.command,"r")) == NULL){
        fprintf(stderr, "[%d] - Thread -- Popen hiba!\n", myData->id);
        exit(1);
    }

    //Kliens uzenetsor megnyitasa
    char clientName[15];
    sprintf(clientName,"/kliens_%d",myData->msg.pid);
    Answer answer;
    mqd_t c_mq_id = mq_open(clientName, O_WRONLY);  

    //Eredmeny kiolvasas es visszakuldese
    char tempLine[256];
    while(fgets(tempLine,100, ansFile) != NULL)
    {   
        tempLine[strlen(tempLine) - 1] = '\0';
        strcpy(answer.line, tempLine);

        //POPEN valaszokat soronkent kuldom a cliensnke
        mq_send(c_mq_id, (char*)&answer, sizeof(Answer), 0);
    }

    /** Az utolso elkuldott uzenet az a Vege szo, hogy a kliens tujda, hogy mikor alljon le**/
    strcpy(answer.line, "END_OF_ANSWER");
    mq_send(c_mq_id, (char*)&answer, sizeof(Answer), 0);
    
    fprintf(stdout, "[%d] - Thread befejezte!\n", myData->id);
    clean(-1, c_mq_id, "", myData->msg.pid);
    return (void*) 0;

}

int main(int argc, char* argv[]){

    //Szerver uzenetsor megnyitasa
    char serverName[30];
    sprintf(serverName, "%s%s", qname, getlogin());

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Msg);

    mqd_t qID = mq_open(serverName, O_RDONLY | O_CREAT, 0600, &attr);
    if(qID  < 0){
        fprintf(stderr, "[Server] - Nem sikerult megnyitni az uzenetsort!\n");
        exit(1);
    }
    else{
        fprintf(stderr, "[Server] - Uzenetsor megnyitva!\n");
    }

    //Szalak elokeszitese
    pthread_t* th = (pthread_t*)malloc(MAXCLIENT * sizeof(pthread_t));
    data* clientData = (data*)malloc(MAXCLIENT * sizeof(data));

    Msg clientRequest;

    int nrClient = 0;
    while(nrClient < MAXCLIENT){
        //Minden keres eseten egy szalat indit, annak lekezelesere
        if(mq_receive(qID, (char*)& clientRequest, sizeof(clientRequest), 0) != -1){
            fprintf(stdout, "[Server] - Command ->%s<- from [%d] - Client -\n",clientRequest.command, clientRequest.pid);

            if(!strcmp(clientRequest.command, "STOP"))
            {
                fprintf(stdout, "[Server] - A szerver leall!\n");
                break;
            }

            clientData[nrClient].msg = clientRequest;
            clientData[nrClient].id = nrClient;
            pthread_create(&th[nrClient], NULL, routine, &clientData[nrClient]);
            nrClient++;

        }
        else{
            clean(qID,-1,"",getpid());
            exit(1);
        }
    }

    for (int i = 0; i < nrClient; i++)
    {
        pthread_join(th[i], NULL);       
    }

    clean(qID, -1, serverName, 0);
    free(clientData);
    free(th);
    exit(0);

}
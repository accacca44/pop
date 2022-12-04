/** Nev: Jako Daniel **/
/** Azonosito: jdim2141 **/
/** Csoport: 522/1 **/
/** Feladat: Lab5/2 **/

#include "msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>

/** Ez a program gondoskodik a szerver bezarasarol, amennyiben az nem allt meg mas okok miatt **/

int main(int argc, char** argv)
{
    int pid = getpid();

    /** Szerver uzenetesoranak a megnyitasa **/
    char sqname[15];
    sprintf(sqname, "%s%s", qname, getlogin());
    mqd_t qid = mq_open(sqname, O_WRONLY);

    if (qid == -1)
    {
        fprintf(stderr, "[%d] - STOP - Mq_open error! - Server_Query!\n", pid);
        exit(1);
    }
 

    Msg msg_stop;
    strcpy(msg_stop.allomanynev, "STOP");
    msg_stop.pid = pid;

    fprintf(stdout, "[%d] - STOP - Sent this filename: %s\n", msg_stop.pid, msg_stop.allomanynev);
    
    /** Az uzenet elkuldese a szerver uzenetsoraba **/
    if (mq_send(qid, (char*)&msg_stop, sizeof(msg_stop), 0) == -1)
    {
        fprintf(stderr, "[%d] - STOP - Mq_send Error -> To Server Query!\n", pid);
        mq_close(qid);
        exit(1);
    }

    /** Szerver uzenetsoranak a bezarasa **/

    if (mq_close(qid) < 0)
    {
        fprintf(stderr, "[%d] - STOP - Mq-Close Error -> Server Query!\n", pid);
    }
    else
    {
        fprintf(stdout, "[%d] - STOP - Server Query Closed Succesfully!\n", pid);
    }

   exit(0);
}
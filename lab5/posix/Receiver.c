#include "Msg.h"
#define _XOPEN_SOURCE 600
#include <time.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char** argv)
{
	if (argc != 2){
    		printf("Usage:  %s /qname\n", argv[0]);
    		exit(1);
	}
 
  	char * name  = argv[1];
  	int flags = O_RDWR;	//flagek beállítása
 
  	mqd_t qid = mq_open(name, flags);	//Üzenetsor megnyitása
  	if (qid <0){
    		perror("Failed to open the message queue");
		exit(1);
  	}

  	bool quit = false;
  	while (!quit){
    		Msg msg = {0,0,0,0};
    		unsigned int priority = 0;  

    		if (mq_receive(qid, (char*) &msg, sizeof(msg), &priority) != -1){ //Üzenet fogadása
      			if (msg.x != -1){      
        			printf("msg = (%03d, %03d, %03d), priority: %d\n", msg.x, msg.y, msg.z, priority);
        			msg.szum = msg.x + msg.y + msg.z;
        			if (mq_send(qid, (char*) &msg, sizeof(Msg), 0) == -1){ //válasz küldése
          				perror("Failed to send msg");
					exit(1);
        			}
      			}
      			else{
        			quit=true;
      			}
    		}
    		else{
      			perror("mq_receive");
      			exit(1);
    		}
  	}
  	mq_close(qid);	//üzenetsor bezárása
  	return 0;
}

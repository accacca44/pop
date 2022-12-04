#include "Msg.h"
#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>   
#include <time.h>     
#include <errno.h>
#include <string.h>
#include <limits.h> // MQ_PRIO_MAX

int main(int argc, char** argv)
{
	if (argc != 2){
		printf("Usage:  %s /qname\n", argv[0]);
    		return 1;
  	}

	char* name  = argv[1];
	int flags = O_RDWR | O_CREAT;	//flagek beállítása
  	mode_t mode  = 0666;	//jogok beállítása

  	struct mq_attr attr;	//struktura létrehozása és beállítása
	attr.mq_flags   = 0;
	attr.mq_maxmsg  = 10;
	attr.mq_msgsize = sizeof(Msg);
	attr.mq_curmsgs = 0;

	mqd_t qid = mq_open(name, flags, mode, &attr);	//Üzenetsor létrehozása 

  	if (qid == -1){   
        	perror("mq_open():");
		exit(1);	
	}

  	struct mq_attr temp;   

  	if (mq_getattr( qid, &temp) == -1 ){	//Üzenetsor tulajdonságainak lekérdezése 
        	perror("getattr:");
		exit(1);	
	}
  	else                         
	        printf("maxmsg = %d\n", temp.mq_maxmsg); 

  	srandom(time(0));
	printf("Press enter to send a message...");
  	getchar();
	char c;
  	do
  	{
		Msg msg = {random() % 100, random() % 100, random() % 100, 0};
		int prio = random() % MQ_PRIO_MAX;  //Véletlenszerűen generálja az üzenet tartalmát és prioritását
		printf("sending msg = (%03d, %03d, %03d) with priority: %d\n", msg.x, msg.y, msg.z,prio);
	        if (mq_send(qid, (char*) &msg, sizeof(Msg), prio) == -1) //üzenet küldése
	        {
	        	perror("Failed to send msg");
			exit(1);	
	        }
		else
		{
			Msg msgr = {0,0,0,0};
        		if (mq_receive(qid, (char*) &msgr, sizeof(msgr), 0) != -1)  //válasz fogadása
        		{
              			printf("szum = %d\n", msgr.szum);
        		}
        		else
        		{
	                	perror("mq_receive");
	 			return(1);
		        }
			printf("Press enter to send another message...");
		        c = getchar();

  		}
	} while (c != 'q');

	Msg msg = {-1,0,0};	//-1 el kezdődő üzenettel jellezuk, hogy készen vagyunk            
	if (mq_send(qid, (char*) &msg, sizeof(Msg), 0) == -1)
	{
		perror("Failed to send msg");
		exit(1);
  	}

  	mq_close(qid);	//üzenetsor bezárása         
  	mq_unlink(argv[1]);	//üzenetsor törlése          
  	
	return 0;
}

#include "mes.h"	/* a közös mes állomány */

int msgid;	/* üzenetsor azonosítója */

void do_it(MESSAGE *mesp)	/* tev.kód -> tev.elnevezés */
{
	switch (mesp->cod_op)	/* a kliens által küldött kód */
	{
		case READ:	/* READ = 1 tev. elnevezése */
			strcpy(mesp->mtext, "READ\n");
			break;
		case WRITE:	/* WRITE = 2 tev. elnevezése */
			strcpy(mesp->mtext, "WRITE\n");
      			break;
    		default:	/* ismeretlen tev.kód */
      			strcpy(mesp->mtext, "Ismeretlen\n");
      			break;
  	}
}

void main(void)
{
	MESSAGE mesp;	/* üzenet */
	if ((msgid = msgget((key_t) KEY, IPC_CREAT | 0666)) < 0){
		perror("In msgget()");	/* üzenetsor létrehozása */
		exit(1);
	}

	while(1)	/* végtelenciklus */
	{
		if (msgrcv(msgid, &mesp, sizeof(mesp)-sizeof(long), 1L, 0) < 0){
			perror("In msgrcv()");	/* tev.kód kiolvasása a sorból */
			exit(1);		
		}

		do_it(&mesp);	/* átalakítás */

		mesp.mtip = mesp.pid;	/* kliens kódja = üzenet típusa */
		mesp.pid = getpid();

		if (msgsnd(msgid, &mesp, sizeof(mesp)-sizeof(long), 0) < 0){
			perror("In msgsnd()");	/* tev. elnevezés küldése */
			exit(1);		
		}
	}
}

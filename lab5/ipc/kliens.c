#include "mes.h"	/* a közös mes állomány */

int msgid;	/* üzenetsor azonosítója */

void main(int argc, char **argv)
{
	pid_t pid;	/* folyamatazonosító */
	MESSAGE mesp;	/* üzenet */

	if (argc != 2){	/* hibás argumentumok */
		printf("hasznalat: c <cop_op>\n");
		exit(1);	
	}
	if ((msgid = msgget((key_t) KEY, 0666)) < 0){
		perror("In msgget()");	/* üzenetsor ID-jánek lekérdezése */
		exit(1);
	}

	mesp.mtip = 1;	/* a kérés típusa mindig 1 */
	mesp.cod_op = atoi(argv[1]);	/* tevékenység kódja parancssorból */
	pid = mesp.pid = getpid();
	printf("a %d kliens elkuldte a kerest\n", mesp.pid);

	if (msgsnd(msgid, (struct msgbuf *)&mesp, sizeof(mesp)-sizeof(long), 0) < 0){
		perror("In msgsnd()");	/* tev.kód elküldése a szervernek */
		exit(1);	
	}

	if (msgrcv(msgid, (struct msgbuf *)&mesp, sizeof(mesp)-sizeof(long), pid, 0) < 0){
		perror("In msgrcv()");	/* tev.név fogadása a szervertől */
		exit(1);
	}

	mesp.mtext[12] = '\0';	/* eredmény előkészítése, kiíratás */

	printf("a %d szerver a %d kliensnek %s kerest kuldott\n", mesp.pid, pid, mesp.mtext);
}

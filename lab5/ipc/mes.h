#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY 958320	/* az üzenetsor kulcsa */

#define READ 1	/* tevékenységkódok elnevezései */
#define WRITE 2

typedef struct{	/* az üzenet struktúrája */
	long mtip;	/* üzenet típusa */
	int pid;	/* küldő folyamat azonosítója */
	int cod_op;	/* kliens üzenete; tevékenységkód */
	char mtext[13];	/* szerver üzenete; tev. elnevezése */
} MESSAGE;

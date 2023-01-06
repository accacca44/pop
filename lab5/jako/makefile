all: sz kl stop

sz: szerver_akos.c msg_akos.h
	gcc -Wall -o sz szerver_akos.c -lrt -lpthread
kl: kliens_akos.c msg_akos.h
	gcc -Wall -o kl kliens_akos.c -lrt -lpthread
stop: stop_akos.c msg_akos.h
	gcc -Wall -o stop stop_akos.c -lrt -lpthread
clean:
	rm -f kl sz stop

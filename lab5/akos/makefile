all: sz kl

sz: szerver.c msg.h
	gcc -Wall -o sz szerver.c -lrt -lpthread
kl: kliens.c msg.h
	gcc -Wall -o kl kliens.c -lrt -lpthread
clean:
	rm -f kl sz

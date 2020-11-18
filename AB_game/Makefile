CFLAGS=-Wall -pedantic -std=gnu99 -g

.PHONY: clean all

all: 2310A 2310B 2310dealer

player.o: player.c player.h
	gcc $(CFLAGS) -c player.c -o player.o

2310A: player.o 2310A.c
	gcc $(CFLAGS) player.o 2310A.c -o 2310A

2310B: player.o 2310B.c
	gcc $(CFLAGS) player.o 2310B.c -o 2310B

2310dealer: 2310dealer.c player.o
	gcc $(CFLAGS) 2310dealer.c player.o -o 2310dealer

clean: 
	rm -rf player.o 2310dealer 2310A 2310B

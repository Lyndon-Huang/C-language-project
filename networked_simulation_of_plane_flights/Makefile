CFLAGS=-Wall -pedantic -std=gnu99 -g -pthread

.PHONY: clean all

all: mapper2310 control2310 roc2310

mapper2310: tool.h mapper2310.c
	gcc $(CFLAGS) mapper2310.c -o mapper2310

control2310: tool.h control2310.c
	gcc $(CFLAGS) control2310.c -o control2310

roc2310: tool.h roc2310.c
	gcc $(CFLAGS) roc2310.c -o roc2310

clean: 
	rm -f mapper2310 control2310 roc2310

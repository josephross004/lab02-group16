CC=gcc
CFLAGS=-c -Wall -g

all: lab02 testcase

lab02: shell.o lab02.o
	$(CC) shell.o lab02.o -o lab02

testcase: testcase.o
	$(CC) shell.o testcase.o -o testcase

shell.o: shell.c
	$(CC) $(CFLAGS) shell.c

lab02.o: lab02.c
	$(CC) $(CFLAGS) lab02.c

clean:
	/bin/rm -f lab02 testcase *.o *.log

run:
	./lab02

test:
	./testharness.sh

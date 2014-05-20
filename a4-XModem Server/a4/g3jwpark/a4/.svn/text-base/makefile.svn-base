CC  = gcc
PORT=55164
CFLAGS= -DPORT=\$(PORT) -g -Wall

all: xmodemserver client1

xmodemserver: xmodemserver.o helper.o crc16.o
	${CC} ${CFLAGS} -o $@ xmodemserver.o helper.o crc16.o

client1: client1.o crc16.o
	${CC} ${CFLAGS} -o $@ client1.o crc16.o

client1.o: client1.c  
	${CC} ${CFLAGS}  -c $^

xmodemserver.o: xmodemserver.c xmodemserver.h 
	${CC} ${CFLAGS}  -c $^

crc16.o: crc16.c crc16.h
	${CC} ${CFLAGS} -c $<

helper.o: helper.c
	${CC} ${CFLAGS} -c $<
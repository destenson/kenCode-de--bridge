CC = gcc
CFLAGS = -D_BSD_SOURCE -O0 -I../include -I../../../ipfs/c-libp2p/include -g3 -Wall
LFLAGS = 
C11 = -std=c11
DEPS = ../include/utils/https.h
OBJS =  https.o jsmn.o json.o logging.o websocket.o

all: $(OBJS)

websocket.o: websocket.c
	$(CC) -c -o websocket.o websocket.c $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(C11) $(CFLAGS)

clean:
	rm -f $(OBJS)

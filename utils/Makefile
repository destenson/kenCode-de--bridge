CC = gcc
CFLAGS = -O0 -I../include -I../../../ipfs/c-libp2p/include -g3 -Wall
LFLAGS = 
DEPS = ../include/utils/https.h
OBJS =  https.o jsmn.o json.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -f *.o

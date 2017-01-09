CC = gcc
CFLAGS = -O0 -I../include -Wall

ifdef DEBUG
CFLAGS += -g3
endif

ifdef SINGLE_THREADED
CFLAGS += -DSINGLE_THREADED
endif

DEPS = ../include/bridge/book.h
OBJS =  book.o mock.o bittrex.o btc38.o poloniex.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

all: $(OBJS)

clean:
	rm -f *.o

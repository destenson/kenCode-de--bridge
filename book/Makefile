CC = gcc
CFLAGS = -O0 -I../include -g3 -Wall
LFLAGS = 
DEPS = ../include/bridge/book.h
OBJS =  book.o mock.o bittrex.o btc38.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -f *.o

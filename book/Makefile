CC = gcc
CFLAGS = -O0 -I../include -g3 -Wall
LFLAGS = 
DEPS = ../include/bridge/book.h ../include/bridge/book_mock.h
OBJS =  book.o mock_book.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -f *.o

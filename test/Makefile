CC = gcc
CFLAGS = -O0 -I../include -g3 -Wall
LFLAGS = 
DEPS = test_book.h
OBJS = testit.o \
	../book/book.o ../book/mock_book.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test_bridge: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) 

all: test_bridge

clean:
	rm -f *.o
	rm -f test_bridge
CC = gcc
CFLAGS = -O0 -I../include -g3 -Wall
LFLAGS = 
DEPS = ../include/utils/https.h
OBJS =  https.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -f *.o

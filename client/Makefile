CC = gcc
CFLAGS = -O0 -Wall
LFLAGS = 

ifdef SINGLE_THREADED
CFLAGS += -DSINGLE_THREADED
else
LFLAGS += -lpthread
endif

ifdef DEBUG
CFLAGS += -g3
endif

DEPS = 
OBJS = cgi_client.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cgi_client: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) 

all: cgi_client

clean:
	rm -f *.o
	rm -f cgi_client
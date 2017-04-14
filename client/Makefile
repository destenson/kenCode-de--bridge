CC = gcc
CFLAGS = -D_BSD_SOURCE -std=c11 -O0 -Wall -I../include
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

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cgi_client: cgi_client.o ../book/market.o ../utils/logging.o ../../../ipfs/c-protobuf/protobuf.o ../../../ipfs/c-protobuf/varint.o /usr/local/lib/libjson-c.a
	$(CC) -o $@ $^ $(LFLAGS)
	
test_client: test_client.o
	$(CC) -o $@ $^ $(LFLAGS)

all: test_client cgi_client

clean:
	rm -f *.o
	rm -f cgi_client

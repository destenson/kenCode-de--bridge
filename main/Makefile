CC = gcc
CFLAGS = -O0 -I../include -I../../../ipfs/c-libp2p/include -Wall
LFLAGS = -L../../../ipfs/c-libp2p -lcurl -lp2p -lm

ifdef SINGLE_THREADED
CFLAGS += -DSINGLE_THREADED
else
LFLAGS += -lpthread
endif

ifdef DEBUG
CFLAGS += -g3
endif

DEPS = 
OBJS = main.o \
	../book/book.o ../book/mock.o ../book/bittrex.o ../book/btc38.o ../book/poloniex.o \
	../utils/https.o ../utils/jsmn.o ../utils/json.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bridge: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) 

all: bridge

clean:
	rm -f *.o
	rm -f bridge
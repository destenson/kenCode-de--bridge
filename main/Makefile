CC = gcc
CFLAGS = -std=c11 -D_BSD_SOURCE -O0 -I../include -I../../../ipfs/c-libp2p/include -Wall
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
	 ../book/market.o ../book/vendor.o \
	../utils/https.o ../utils/jsmn.o ../utils/json.o ../utils/logging.o \
	../../../ipfs/c-protobuf/protobuf.o ../../../ipfs/c-protobuf/varint.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bridge: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) 

all: bridge

clean:
	rm -f *.o
	rm -f bridge

CC = gcc
CFLAGS = -O0 -I../include -Wall
LFLAGS = -lcurl

ifdef SINGLE_THREADED
CFLAGS += -DSINGLE_THREADED
else
LFLAGS += -lpthread
endif

ifdef DEBUG
CFLAGS += -g3
endif

DEPS = test_book.h
OBJS = testit.o \
	../book/book.o ../book/mock.o ../book/bittrex.o ../book/btc38.o ../book/poloniex.o \
	../utils/https.o ../utils/jsmn.o ../utils/json.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test_bridge: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) 

all: test_bridge

clean:
	rm -f *.o
	rm -f test_bridge
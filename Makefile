
DEBUG = true
export DEBUG

UNAME := $(shell uname)

ifeq ($(UNAME),Darwin)

SINGLE_THREADED = false
export SINGLE_THREADED

endif

all:
	cd book; make all;
	cd utils; make all;
	cd test; make all;
	cd main; make all;
	
clean:
	cd book; make clean;
	cd utils; make clean;
	cd test; make clean;
	cd main; make clean;

rebuild: clean all

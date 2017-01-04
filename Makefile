
DEBUG = true
export DEBUG

all:
	cd book; make all;
	cd utils; make all;
	cd test; make all;
	
clean:
	cd book; make clean;
	cd utils; make clean;
	cd test; make clean;

rebuild: clean all

CFLAGS=-ansi -Wpedantic -Wall -Werror -D_THREAD_SAFE -D_REENTRANT -D_POSIX_C_SOURCE=200112L
LIBRARIES=-lpthread 
LFLAGS=

all: fornaio.exe

fornaio.exe: fornaio.o DBGpthread.o
	gcc ${LFLAGS} -o fornaio.exe fornaio.o DBGpthread.o ${LIBRARIES}

fornaio.o: fornaio.c DBGpthread.h
	gcc -c ${CFLAGS} fornaio.c 

DBGpthread.o: DBGpthread.c printerror.h
	gcc -c ${CFLAGS} DBGpthread.c

.PHONY: clean

clean: 
	rm -f *.exe *.o *~ core

run: fornaio.exe
	./fornaio.exe   

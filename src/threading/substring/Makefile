CCFLAGS = -Wall --std=c99 -lpthread

default: thread

debug: CCFLAGS += -g
debug: thread

thread: substring.c
	gcc -o thread substring.c $(CCFLAGS)

rebuild: clean thread


clean: 
	rm -f a.out thread *.o

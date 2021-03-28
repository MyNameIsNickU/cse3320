CCFLAGS = -Wall --std=c99 -lpthread

default: queue

debug: CCFLAGS += -g
debug: queue

queue: consumer.c
	gcc -o queue consumer.c $(CCFLAGS)

rebuild: clean queue


clean:
	rm -f a.out queue *.o

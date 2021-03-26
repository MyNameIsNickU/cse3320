CCFLAGS = -Wall --std=c99 -lpthread

default: consumer

debug: CCFLAGS += -g
debug: consumer

consumer: consumer.c
	gcc -o consumer consumer.c $(CCFLAGS)

rebuild: clean consumer


clean:
	rm -f a.out consumer *.o

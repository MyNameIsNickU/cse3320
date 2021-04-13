CCFLAGS = -Wall --std=c99

default: fat32

debug: CCFLAGS += -g
debug: fat32

fat32: mfs.c
	gcc -o fat32 mfs.c $(CCFLAGS)

rebuild: clean fat32


clean: 
	rm -f a.out fat32 *.o

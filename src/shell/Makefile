CCFLAGS = -Wall --std=c99

default: shell

debug: CCFLAGS += -g
debug: shell

shell: msh.c
	gcc -o shell msh.c $(CCFLAGS)

rebuild: clean shell


clean: 
	rm -f a.out shell *.o

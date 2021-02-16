CCFLAGS = --std=c99

default: shell

shell: msh.c
	gcc -o shell msh.c $(CCFLAGS)

rebuild: clean shell


clean: 
	rm -f a.out shell *.o

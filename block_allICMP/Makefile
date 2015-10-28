block_allICMP: block_allICMP.o divertlib.o
	gcc block_allICMP.o divertlib.o -o block_allICMP
block_allICMP.o: block_allICMP.c
	gcc -c block_allICMP.c -o block_allICMP.o
divertlib.o: divertlib.c divertlib.h
	gcc -c divertlib.c divertlib.h

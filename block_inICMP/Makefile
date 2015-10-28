block_inICMP: block_inICMP.o divertlib.o
	gcc block_inICMP.o divertlib.o -o block_inICMP
block_inICMP.o: block_inICMP.c
	gcc -c block_inICMP.c -o block_inICMP.o
divertlib.o: divertlib.c divertlib.h
	gcc -c divertlib.c divertlib.h

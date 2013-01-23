# Makefile for Red/Blue simulator

CC = gcc
CCFLAGS = -std=c99 -g -pedantic -Wall 

both: orbs.c
	make orbs
	make nomp

orbs: orbs.c
	$(CC) $(CCFLAGS) -fopenmp -o orbs orbs.c

nomp: orbs.c
	$(CC) $(CCFLAGS)  -o norbs orbs.c

clean:
	rm orbs.exe norbs.exe
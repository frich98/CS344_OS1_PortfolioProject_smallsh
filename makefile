# Makefile for CS344 - Assignment 3 - smallsh
# Frannie Richert

CC = gcc
CFLAGS = --std=gnu99
SRCEXT = c

movies: main.c
	$(CC) $(CFLAGS) -o smallsh main.c cmd_functions.c cmd_functions.h process_functions.c process_functions.h

clean : 
	rm smallsh
# makefile for channels examples
# use: make [producer|pipeline|clean]

# If you create further source files, add them to the following line
# (separated by spaces).
SRC=channels.c

use:
	@echo "Use: make [producer|pipeline|clean]"

CC=gcc -g -std=gnu99 -Wall -Werror
LIB=-lpthread -lrt -pthread

producer: channels.o producer.c
	$(CC) $(LIB) channels.o producer.c -o producer

pipeline: channels.o pipeline.c
	$(CC) $(LIB) channels.o pipeline.c -o pipeline

channels.o: $(SRC) channels.h
	$(CC) channels.h $(SRC) -c

clean:
	rm producer pipeline *.o

CC = gcc
CFLAGS = -Wall -Werror -ggdb

LD =
LD += -lncursesw
OBJECTS =
OBJECTS += chunk_node.o
OBJECTS += chunk.o

all: curses

curses: curses.c $(OBJECTS)
	$(CC) $(CFLAGS) $(LD) -o $@ $(OBJECTS) $<

chunk.o: LD = -lm

%.o: %.c %.h
	$(CC) $(CFLAGS) $(LD) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f curses

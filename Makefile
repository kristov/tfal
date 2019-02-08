CC = gcc
CFLAGS = -Wall -Werror -ggdb

OBJECTS =
OBJECTS += ast.o
OBJECTS += chunk.o

all: test

test: test.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f test

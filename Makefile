CC = gcc
CFLAGS = -Wall -Werror -ggdb

LD =
OBJECTS =
OBJECTS += ast.o
OBJECTS += chunk.o

all: test

test: test.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $<

chunk.o: LD = -lm

%.o: %.c %.h
	$(CC) $(CFLAGS) $(LD) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f test

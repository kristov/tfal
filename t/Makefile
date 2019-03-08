CC = gcc
CFLAGS = -Wall -Werror -ggdb

TESTS =
TESTS += test_chunk.t
TESTS += test_chunk_build.t
TESTS += test_chunk_node.t

all: test_harness.o $(TESTS)

test_chunk.t: OBJECTS = ../chunk.o
test_chunk_build.t: OBJECTS = ../chunk.o
test_chunk_node.t: OBJECTS = ../chunk.o ../chunk_node.o

%.t: %.c
	$(CC) $(CFLAGS) -o $@ test_harness.o $(OBJECTS) $<

test_harness.o: test_harness.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TESTS)

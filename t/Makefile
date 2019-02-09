CC = gcc
CFLAGS = -Wall -Werror -ggdb

TESTS =
TESTS += test_chunk.t

all: test_harness.o $(TESTS)

%.t: %.c
	$(CC) $(CFLAGS) -o $@ test_harness.o ../chunk.o $<

test_harness.o: test_harness.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TESTS)

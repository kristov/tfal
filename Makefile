CC := gcc
CFLAGS := -Wall -Werror -ggdb

all: memlen chunk

memlen: memlen.c
	$(CC) $(CFLAGS) -o $@ $<

chunk: chunk.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f memlen
